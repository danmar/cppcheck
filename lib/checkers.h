/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

namespace checkers {

    static std::map<std::string, std::string> allCheckers{
        {"CheckBool::checkIncrementBoolean","style"},
        {"CheckBool::checkBitwiseOnBoolean","style,inconclusive"},
        {"CheckBool::checkComparisonOfBoolWithInt","warning,c++"},
        {"CheckBool::checkComparisonOfFuncReturningBool","style,c++"},
        {"CheckBool::checkComparisonOfBoolWithBool","style,c++"},
        {"CheckBool::checkAssignBoolToPointer",""},
        {"CheckBool::checkComparisonOfBoolExpressionWithInt","warning"},
        {"CheckBool::pointerArithBool",""},
        {"CheckBool::checkAssignBoolToFloat","style,c++"},
        {"CheckBool::returnValueOfFunctionReturningBool","style"},
        {"CheckPostfixOperator::postfixOperator","performance"},
        {"CheckSizeof::checkSizeofForNumericParameter","warning"},
        {"CheckSizeof::checkSizeofForArrayParameter","warning"},
        {"CheckSizeof::checkSizeofForPointerSize","warning"},
        {"CheckSizeof::sizeofsizeof","warning"},
        {"CheckSizeof::sizeofCalculation","warning"},
        {"CheckSizeof::sizeofFunction","warning"},
        {"CheckSizeof::suspiciousSizeofCalculation","warning,inconclusive"},
        {"CheckSizeof::sizeofVoid","portability"},
        {"Check64BitPortability::pointerassignment","portability"},
        {"CheckStl::outOfBounds",""},
        {"CheckStl::outOfBoundsIndexExpression",""},
        {"CheckStl::iterators",""},
        {"CheckStl::misMatchingContainers",""},
        {"CheckStl::misMatchingContainerIterator",""},
        {"CheckStl::invalidContainer",""},
        {"CheckStl::stlOutOfBounds",""},
        {"CheckStl::negativeIndex",""},
        {"CheckStl::erase",""},
        {"CheckStl::stlBoundaries",""},
        {"CheckStl::if_find","warning,performance"},
        {"CheckStl::checkFindInsert","performance"},
        {"CheckStl::size","performance,c++03"},
        {"CheckStl::redundantCondition","style"},
        {"CheckStl::missingComparison","warning"},
        {"CheckStl::string_c_str",""},
        {"CheckStl::uselessCalls","performance,warning"},
        {"CheckStl::checkDereferenceInvalidIterator","warning"},
        {"CheckStl::checkDereferenceInvalidIterator2",""},
        {"CheckStl::useStlAlgorithm","style"},
        {"CheckStl::knownEmptyContainer","style"},
        {"CheckStl::checkMutexes","warning"},
        {"CheckBoost::checkBoostForeachModification",""},
        {"CheckNullPointer::nullPointer",""},
        {"CheckNullPointer::nullConstantDereference",""},
        {"CheckNullPointer::aithmetic",""},
        {"CheckNullPointer::analyseWholeProgram","unusedfunctions"},
        {"CheckBufferOverrun::arrayIndex",""},
        {"CheckBufferOverrun::pointerArithmetic","portability"},
        {"CheckBufferOverrun::bufferOverflow",""},
        {"CheckBufferOverrun::arrayIndexThenCheck",""},
        {"CheckBufferOverrun::stringNotZeroTerminated","warning,inconclusive"},
        {"CheckBufferOverrun::argumentSize","warning"},
        {"CheckBufferOverrun::analyseWholeProgram",""},
        {"CheckBufferOverrun::objectIndex",""},
        {"CheckBufferOverrun::negativeArraySize",""},
        {"CheckUninitVar::check",""},
        {"CheckUninitVar::valueFlowUninit",""},
        {"CheckOther::checkCastIntToCharAndBack","warning"},
        {"CheckOther::clarifyCalculation","style"},
        {"CheckOther::clarifyStatement","warning"},
        {"CheckOther::checkSuspiciousSemicolon","warning,inconclusive"},
        {"CheckOther::warningOldStylePointerCast","style,c++"},
        {"CheckOther::invalidPointerCast","portability"},
        {"CheckOther::checkRedundantAssignment","style"},
        {"CheckOther::redundantBitwiseOperationInSwitch","warning"},
        {"CheckOther::checkSuspiciousCaseInSwitch","warning,inconclusive"},
        {"CheckOther::checkUnreachableCode","style"},
        {"CheckOther::checkVariableScope","style,notclang"},
        {"CheckOther::checkPassByReference","performance,c++"},
        {"CheckOther::checkConstPointer","style"},
        {"CheckOther::checkCharVariable","warning,portability"},
        {"CheckOther::checkIncompleteStatement","warning"},
        {"CheckOther::checkZeroDivision",""},
        {"CheckOther::checkNanInArithmeticExpression","style"},
        {"CheckOther::checkMisusedScopedObject","style,c++"},
        {"CheckOther::checkDuplicateBranch","style,inconclusive"},
        {"CheckOther::checkInvalidFree",""},
        {"CheckOther::checkDuplicateExpression","style,warning"},
        {"CheckOther::checkComparisonFunctionIsAlwaysTrueOrFalse","warning"},
        {"CheckOther::checkSignOfUnsignedVariable","style"},
        {"CheckOther::checkRedundantCopy","c++,performance,inconclusive"},
        {"CheckOther::checkNegativeBitwiseShift",""},
        {"CheckOther::checkIncompleteArrayFill","warning,portability,inconclusive"},
        {"CheckOther::checkVarFuncNullUB","portability"},
        {"CheckOther::checkRedundantPointerOp","style"},
        {"CheckOther::checkInterlockedDecrement","windows-platform"},
        {"CheckOther::checkUnusedLabel","style,warning"},
        {"CheckOther::checkEvaluationOrder","C/C++03"},
        {"CheckOther::checkAccessOfMovedVariable","c++11,warning"},
        {"CheckOther::checkFuncArgNamesDifferent","style,warning,inconclusive"},
        {"CheckOther::checkShadowVariables","style"},
        {"CheckOther::checkKnownArgument","style"},
        {"CheckOther::checkKnownPointerToBool","style"},
        {"CheckOther::checkComparePointers",""},
        {"CheckOther::checkModuloOfOne","style"},
        {"CheckOther::checkOverlappingWrite",""},
        {"CheckClass::checkConstructors","style,warning"},
        {"CheckClass::checkExplicitConstructors","style"},
        {"CheckClass::checkCopyConstructors","warning"},
        {"CheckClass::initializationListUsage","performance"},
        {"CheckClass::privateFunctions","style"},
        {"CheckClass::checkMemset",""},
        {"CheckClass::operatorEqRetRefThis","style"},
        {"CheckClass::operatorEqToSelf","warning"},
        {"CheckClass::virtualDestructor",""},
        {"CheckClass::thisSubtraction","warning"},
        {"CheckClass::checkConst","style,inconclusive"},
        {"CheckClass::initializerListOrder","style,inconclusive"},
        {"CheckClass::checkSelfInitialization",""},
        {"CheckClass::checkVirtualFunctionCallInConstructor","warning"},
        {"CheckClass::checkDuplInheritedMembers","warning"},
        {"CheckClass::checkMissingOverride","style,c++03"},
        {"CheckClass::checkUselessOverride","style"},
        {"CheckClass::checkThisUseAfterFree","warning"},
        {"CheckClass::checkUnsafeClassRefMember","warning,safeChecks"},
        {"CheckClass::analyseWholeProgram",""},
        {"CheckUnusedVar::checkFunctionVariableUsage","style"},
        {"CheckUnusedVar::checkStructMemberUsage","style"},
        {"CheckIO::checkCoutCerrMisusage","c"},
        {"CheckIO::checkFileUsage",""},
        {"CheckIO::checkWrongPrintfScanfArguments",""},
        {"CheckCondition::assignIf","style"},
        {"CheckCondition::checkBadBitmaskCheck","style"},
        {"CheckCondition::comparison","style"},
        {"CheckCondition::duplicateCondition","style"},
        {"CheckCondition::multiCondition","style"},
        {"CheckCondition::multiCondition2","warning"},
        {"CheckCondition::checkIncorrectLogicOperator","style,warning"},
        {"CheckCondition::checkModuloAlwaysTrueFalse","warning"},
        {"CheckCondition::clarifyCondition","style"},
        {"CheckCondition::alwaysTrueFalse","style"},
        {"CheckCondition::checkInvalidTestForOverflow","warning"},
        {"CheckCondition::checkPointerAdditionResultNotNull","warning"},
        {"CheckCondition::checkDuplicateConditionalAssign","style"},
        {"CheckCondition::checkAssignmentInCondition","style"},
        {"CheckCondition::checkCompareValueOutOfTypeRange","style,platform"},
        {"CheckFunctions::checkProhibitedFunctions",""},
        {"CheckFunctions::invalidFunctionUsage",""},
        {"CheckFunctions::checkIgnoredReturnValue","style,warning"},
        {"CheckFunctions::checkMissingReturn",""},
        {"CheckFunctions::checkMathFunctions","style,warning,c99,c++11"},
        {"CheckFunctions::memsetZeroBytes","warning"},
        {"CheckFunctions::memsetInvalid2ndParam","warning,portability"},
        {"CheckFunctions::returnLocalStdMove","performance,c++11"},
        {"CheckFunctions::useStandardLibrary","style"},
        {"CheckVaarg::va_start_argument",""},
        {"CheckVaarg::va_list_usage","notclang"},
        {"CheckUnusedFunctions::analyseWholeProgram","unusedFunctions"},
        {"CheckType::checkTooBigBitwiseShift","platform"},
        {"CheckType::checkIntegerOverflow","platform"},
        {"CheckType::checkSignConversion","warning"},
        {"CheckType::checkLongCast","style"},
        {"CheckType::checkFloatToIntegerOverflow",""},
        {"CheckString::stringLiteralWrite",""},
        {"CheckString::checkAlwaysTrueOrFalseStringCompare","warning"},
        {"CheckString::checkSuspiciousStringCompare","warning"},
        {"CheckString::strPlusChar",""},
        {"CheckString::checkIncorrectStringCompare","warning"},
        {"CheckString::overlappingStrcmp","warning"},
        {"CheckString::sprintfOverlappingData",""},
        {"CheckAssert::assertWithSideEffects","warning"},
        {"CheckExceptionSafety::destructors","warning"},
        {"CheckExceptionSafety::deallocThrow","warning"},
        {"CheckExceptionSafety::checkRethrowCopy","style"},
        {"CheckExceptionSafety::checkCatchExceptionByValue","style"},
        {"CheckExceptionSafety::nothrowThrows",""},
        {"CheckExceptionSafety::unhandledExceptionSpecification","style,inconclusive"},
        {"CheckExceptionSafety::rethrowNoCurrentException",""},
        {"CheckAutoVariables::assignFunctionArg","style,warning"},
        {"CheckAutoVariables::autoVariables",""},
        {"CheckAutoVariables::checkVarLifetime",""},
        {"CheckLeakAutoVar::check","notclang"},
        {"CheckMemoryLeakInFunction::checkReallocUsage",""},
        {"CheckMemoryLeakInClass::check",""},
        {"CheckMemoryLeakStructMember::check",""},
        {"CheckMemoryLeakNoVar::check",""},
    };


    static std::map<std::string, std::string> premiumCheckers{
        {"PremiumCheckBufferOverrun::addressOfPointerArithmetic","warning"},
        {"PremiumCheckBufferOverrun::negativeBufferSizeCheckedNonZero","warning"},
        {"PremiumCheckBufferOverrun::negativeBufferSizeCheckedNonZero","warning"},
        {"PremiumCheckHang::infiniteLoop",""},
        {"PremiumCheckHang::infiniteLoopContinue",""},
        {"PremiumCheckOther::arrayPointerComparison","style"},
        {"PremiumCheckOther::knownResult","style"},
        {"PremiumCheckOther::lossOfPrecision","style"},
        {"PremiumCheckOther::pointerCast","style"},
        {"PremiumCheckOther::reassignInLoop","style"},
        {"PremiumCheckOther::unreachableCode","style"},
        {"PremiumCheckUninitVar::uninitvar",""},
        {"PremiumCheckUninitVar::uninitmember",""},
        {"PremiumCheckUnusedVar::unreadVariable","style"},
        {"PremiumCheckUnusedVar::unusedPrivateMember","style"},
    };



    struct MisraInfo {
        int a;
        int b;
        const char* str;
        int amendment;
    };

    const char Req[] = "Required";
    const char Adv[] = "Advisory";
    const char Man[] = "Mandatory";

    const MisraInfo misraC2012Rules[] =
    {
        {1,1,Req,0},
        {1,2,Adv,0},
        {1,3,Req,0},
        {1,4,Req,2}, // amendment 2
        {1,5,Req,3}, // Amendment 3
        {2,1,Req,0},
        {2,2,Req,0},
        {2,3,Adv,0},
        {2,4,Adv,0},
        {2,5,Adv,0},
        {2,6,Adv,0},
        {2,7,Adv,0},
        {2,8,Adv,0},
        {3,1,Req,0},
        {3,2,Req,0},
        {4,1,Req,0},
        {4,2,Adv,0},
        {5,1,Req,0},
        {5,2,Req,0},
        {5,3,Req,0},
        {5,4,Req,0},
        {5,5,Req,0},
        {5,6,Req,0},
        {5,7,Req,0},
        {5,8,Req,0},
        {5,9,Adv,0},
        {6,1,Req,0},
        {6,2,Req,0},
        {6,3,Req,0},
        {7,1,Req,0},
        {7,2,Req,0},
        {7,3,Req,0},
        {7,4,Req,0},
        {7,5,Man,0},
        {7,6,Req,0},
        {8,1,Req,0},
        {8,2,Req,0},
        {8,3,Req,0},
        {8,4,Req,0},
        {8,5,Req,0},
        {8,6,Req,0},
        {8,7,Adv,0},
        {8,8,Req,0},
        {8,9,Adv,0},
        {8,10,Req,0},
        {8,11,Adv,0},
        {8,12,Req,0},
        {8,13,Adv,0},
        {8,14,Req,0},
        {8,15,Req,0},
        {8,16,Adv,0},
        {8,17,Adv,0},
        {9,1,Man,0},
        {9,2,Req,0},
        {9,3,Req,0},
        {9,4,Req,0},
        {9,5,Req,0},
        {9,6,Req,0},
        {9,7,Man,0},
        {10,1,Req,0},
        {10,2,Req,0},
        {10,3,Req,0},
        {10,4,Req,0},
        {10,5,Adv,0},
        {10,6,Req,0},
        {10,7,Req,0},
        {10,8,Req,0},
        {11,1,Req,0},
        {11,2,Req,0},
        {11,3,Req,0},
        {11,4,Adv,0},
        {11,5,Adv,0},
        {11,6,Req,0},
        {11,7,Req,0},
        {11,8,Req,0},
        {11,9,Req,0},
        {11,10,Req,0},
        {12,1,Adv,0},
        {12,2,Req,0},
        {12,3,Adv,0},
        {12,4,Adv,0},
        {12,5,Man,1}, // amendment 1
        {12,6,Req,4}, // amendment 4
        {13,1,Req,0},
        {13,2,Req,0},
        {13,3,Adv,0},
        {13,4,Adv,0},
        {13,5,Req,0},
        {13,6,Man,0},
        {14,1,Req,0},
        {14,2,Req,0},
        {14,3,Req,0},
        {14,4,Req,0},
        {15,1,Adv,0},
        {15,2,Req,0},
        {15,3,Req,0},
        {15,4,Adv,0},
        {15,5,Adv,0},
        {15,6,Req,0},
        {15,7,Req,0},
        {16,1,Req,0},
        {16,2,Req,0},
        {16,3,Req,0},
        {16,4,Req,0},
        {16,5,Req,0},
        {16,6,Req,0},
        {16,7,Req,0},
        {17,1,Req,0},
        {17,2,Req,0},
        {17,3,Man,0},
        {17,4,Man,0},
        {17,5,Adv,0},
        {17,6,Man,0},
        {17,7,Req,0},
        {17,8,Adv,0},
        {17,9,Man,0},
        {17,10,Req,0},
        {17,11,Adv,0},
        {17,12,Adv,0},
        {17,13,Req,0},
        {18,1,Req,0},
        {18,2,Req,0},
        {18,3,Req,0},
        {18,4,Adv,0},
        {18,5,Adv,0},
        {18,6,Req,0},
        {18,7,Req,0},
        {18,8,Req,0},
        {18,9,Req,0},
        {18,10,Man,0},
        {19,1,Man,0},
        {19,2,Adv,0},
        {20,1,Adv,0},
        {20,2,Req,0},
        {20,3,Req,0},
        {20,4,Req,0},
        {20,5,Adv,0},
        {20,6,Req,0},
        {20,7,Req,0},
        {20,8,Req,0},
        {20,9,Req,0},
        {20,10,Adv,0},
        {20,11,Req,0},
        {20,12,Req,0},
        {20,13,Req,0},
        {20,14,Req,0},
        {21,1,Req,0},
        {21,2,Req,0},
        {21,3,Req,0},
        {21,4,Req,0},
        {21,5,Req,0},
        {21,6,Req,0},
        {21,7,Req,0},
        {21,8,Req,0},
        {21,9,Req,0},
        {21,10,Req,0},
        {21,11,Req,0},
        {21,12,Adv,0},
        {21,13,Man,1}, // Amendment 1
        {21,14,Req,1}, // Amendment 1
        {21,15,Req,1}, // Amendment 1
        {21,16,Req,1}, // Amendment 1
        {21,17,Req,1}, // Amendment 1
        {21,18,Man,1}, // Amendment 1
        {21,19,Man,1}, // Amendment 1
        {21,20,Man,1}, // Amendment 1
        {21,21,Req,3}, // Amendment 3
        {21,22,Man,3}, // Amendment 3
        {21,23,Req,3}, // Amendment 3
        {21,24,Req,3}, // Amendment 3
        {21,25,Req,4}, // Amendment 4
        {21,26,Req,4}, // Amendment 4
        {22,1,Req,0},
        {22,2,Man,0},
        {22,3,Req,0},
        {22,4,Man,0},
        {22,5,Man,0},
        {22,6,Man,0},
        {22,7,Req,1}, // Amendment 1
        {22,8,Req,1}, // Amendment 1
        {22,9,Req,1}, // Amendment 1
        {22,10,Req,1}, // Amendment 1
        {22,11,Req,4}, // Amendment 4
        {22,12,Man,4}, // Amendment 4
        {22,13,Req,4}, // Amendment 4
        {22,14,Man,4}, // Amendment 4
        {22,15,Req,4}, // Amendment 4
        {22,16,Req,4}, // Amendment 4
        {22,17,Req,4}, // Amendment 4
        {22,18,Req,4}, // Amendment 4
        {22,19,Req,4}, // Amendment 4
        {22,20,Man,4}, // Amendment 4
        {23,1,Adv,3}, // Amendment 3
        {23,2,Req,3}, // Amendment 3
        {23,3,Adv,3}, // Amendment 3
        {23,4,Req,3}, // Amendment 3
        {23,5,Adv,3}, // Amendment 3
        {23,6,Req,3}, // Amendment 3
        {23,7,Adv,3}, // Amendment 3
        {23,8,Req,3}, // Amendment 3
    };

    static std::map<std::string, std::string> misraRuleSeverity{
        {"1.1", "error"}, //{"syntaxError", "unknownMacro"}},
        {"1.3", "error"}, //most "error"
        {"2.1", "style"}, //{"alwaysFalse", "duplicateBreak"}},
        {"2.2", "style"}, //{"alwaysTrue", "redundantCondition", "redundantAssignment", "redundantAssignInSwitch", "unreadVariable"}},
        {"2.6", "style"}, //{"unusedLabel"}},
        {"2.8", "style"}, //{"unusedVariable"}},
        {"5.3", "style"}, //{"shadowVariable"}},
        {"8.3", "style"}, //{"funcArgNamesDifferent"}}, // inconclusive
        {"8.13", "style"}, //{"constPointer"}},
        {"9.1", "error"}, //{"uninitvar"}},
        {"14.3", "style"}, //{"alwaysTrue", "alwaysFalse", "compareValueOutOfTypeRangeError", "knownConditionTrueFalse"}},
        {"13.2", "error"}, //{"unknownEvaluationOrder"}},
        {"13.6", "style"}, //{"sizeofCalculation"}},
        {"17.4", "error"}, //{"missingReturn"}},
        {"17.5", "warning"}, //{"argumentSize"}},
        {"18.1", "error"}, //{"pointerOutOfBounds"}},
        {"18.2", "error"}, //{"comparePointers"}},
        {"18.3", "error"}, //{"comparePointers"}},
        {"18.6", "error"}, //{"danglingLifetime"}},
        {"19.1", "error"}, //{"overlappingWriteUnion", "overlappingWriteFunction"}},
        {"20.6", "error"}, //{"preprocessorErrorDirective"}},
        {"21.13", "error"}, //{"invalidFunctionArg"}},
        {"21.17", "error"}, //{"bufferAccessOutOfBounds"}},
        {"21.18", "error"}, //{"bufferAccessOutOfBounds"}},
        {"22.1", "error"}, //{"memleak", "resourceLeak", "memleakOnRealloc", "leakReturnValNotUsed", "leakNoVarFunctionCall"}},
        {"22.2", "error"}, //{"autovarInvalidDeallocation"}},
        {"22.3", "error"}, //{"incompatibleFileOpen"}},
        {"22.4", "error"}, //{"writeReadOnlyFile"}},
        {"22.6", "error"}, //{"useClosedFile"}}
    };

}


