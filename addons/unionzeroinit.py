#!/usr/bin/env python3
#
# Detect error-prone zero initialization of unions.

import dataclasses
import cppcheck
import cppcheckdata
from typing import List, Tuple


@dataclasses.dataclass
class Member:
    name: str
    size: int


@dataclasses.dataclass
class Union:
    name: str
    members: List[Member] = dataclasses.field(default_factory=list)

    def largest_member(self):
        return sorted(self.members, key=lambda m: -m.size)[0]

    def is_largest_member_first(self):
        sizes = [m.size for m in self.members]

        has_unknown_sizes = 0 in sizes
        if has_unknown_sizes:
            return True

        return sizes[0] == sorted(sizes, key=lambda s: -s)[0]


def estimate_size_of_type(
    platform: cppcheckdata.Platform, type: str, pointer: bool
) -> int:
    bits = 0
    if pointer:
        bits = platform.pointer_bit
    elif type == "char":
        bits = platform.char_bit
    elif type == "short":
        bits = platform.short_bit
    elif type == "int":
        bits = platform.int_bit
    elif type == "long":
        bits = platform.long_bit
    elif type == "long_long":
        bits = platform.long_long_bit
    else:
        # Fair estimate...
        bits = platform.int_bit
    return bits


def tokat(token: cppcheckdata.Token, offset) -> cppcheckdata.Token:
    at = token.tokAt(offset)
    if at:
        return at

    empty = {"str": ""}
    return cppcheckdata.Token(empty)


def parse_array_length(token) -> int:
    if not tokat(token, 1).str == "[":
        return 1

    nelements = 0
    try:
        nelements = int(tokat(token, 2).str)
    except ValueError:
        return 1

    if not tokat(token, 3).str == "]":
        return 1

    return nelements


def is_zero_initialized(token):
    return (
        tokat(token, 1).str == "="
        and tokat(token, 2).str == "{"
        and (
            tokat(token, 3).str == "}"
            or (tokat(token, 3).str == "0" and tokat(token, 4).str == "}")
        )
    )


def is_pointer(variable: cppcheckdata.Variable) -> bool:
    return variable.nameToken.valueType.pointer and not variable.isArray


def accumulated_member_size(
    data: cppcheckdata.CppcheckData, variable: cppcheckdata.Variable
) -> Tuple[str, int]:
    # Note that cppcheck might not be able to observe all types due to
    # inaccessible include(s).
    if not variable.nameToken.valueType:
        return (None, 0)

    if variable.nameToken.valueType.type == "record":
        if not variable.nameToken.valueType.typeScope:
            return (None, 0)

        nested_variables = variable.nameToken.valueType.typeScope.varlist

        # Circumvent what seems to be a bug in which only the last bitfield has
        # its bits properly assigned.
        has_bitfields = any([v.nameToken.valueType.bits for v in nested_variables])
        if has_bitfields:
            return (variable.nameToken.str, len(nested_variables))

        total_size = 0
        for nested in nested_variables:
            # Avoid potential cyclic members referring to the type currently
            # being traversed.
            if is_pointer(nested):
                total_size += data.platform.pointer_bit
            else:
                _, size = accumulated_member_size(data, nested.nameToken.variable)
                total_size += size
        return (variable.nameToken.str, total_size)

    vt = variable.nameToken.valueType
    if vt.bits:
        size = vt.bits
    else:
        size = estimate_size_of_type(
            data.platform,
            variable.nameToken.valueType.type,
            is_pointer(variable),
        )
    if variable.isArray:
        size *= parse_array_length(variable.nameToken)
    return (variable.nameToken.str, size)


def error_message(u: Union):
    return (
        f"Zero initializing union {u.name} does not guarantee its complete "
        "storage to be zero initialized as its largest member is not declared "
        f"as the first member. Consider making {u.largest_member().name} the "
        "first member or favor memset()."
    )


@cppcheck.checker
def union_zero_init(cfg, data, debug=False):
    unions_by_id = {}

    # Detect union declarations.
    for scope in cfg.scopes:
        if not scope.type == "Union":
            continue

        union = Union(name=scope.className)
        for variable in scope.varlist:
            name, size = accumulated_member_size(data, variable)
            union.members.append(Member(name=name, size=size))
        unions_by_id[scope.Id] = union

    if debug:
        for id, u in unions_by_id.items():
            print(id, u, u.is_largest_member_first(), u.largest_member())

    # Detect problematic union variables.
    for token in cfg.tokenlist:
        if (
            token.valueType
            and token.valueType.typeScopeId in unions_by_id
            and token.isName
            and is_zero_initialized(token)
        ):
            id = token.valueType.typeScopeId
            if not unions_by_id[id].is_largest_member_first():
                cppcheck.reportError(
                    token,
                    "portability",
                    error_message(unions_by_id[id]),
                    "unionzeroinit",
                )
