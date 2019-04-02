
#include <sstream>
#include "type2.h"



static int getValue(const uint8_t *data, size_t dataSize, uint8_t maxValue, bool *done = nullptr)
{
    static size_t pos;    // current "data" position
    static int dataValue; // value extracted from data
    static int ones;      // ones. This variable tracks if we need to add more stuff in "dataValue".

    // Shift more bits from "data" into "dataValue" if needed
    while (pos < dataSize && ones < 0xFFFF) {
        ones = (ones << 8) | 0xff;
        dataValue = (dataValue << 8) | data[pos];
        pos++;
    }

    if (done)
        *done = (ones == 0);

    if (maxValue == 0)
        return 0;

    // Shift out info from "dataValue" using % . Using & and >> would work but then we are limited to "power of 2" max value.
    const int ret = dataValue % maxValue;
    ones /= maxValue;
    dataValue /= maxValue;
    return ret;
}

static std::string generateExpression2_lvalue(const uint8_t *data, size_t dataSize)
{
    return "var" + std::to_string(1 + getValue(data, dataSize, 5));
}

static std::string generateExpression2_Op(const uint8_t *data, size_t dataSize, int numberOfGlobalConstants)
{
    std::ostringstream code;
    switch (getValue(data, dataSize, 3)) {
    case 0:
        code << generateExpression2_lvalue(data, dataSize);
        break;
    case 1:
        code << "globalconstant" << (1 + getValue(data, dataSize, numberOfGlobalConstants));
        break;
    case 2:
        code << (getValue(data, dataSize, 0x80) * 0x80 + getValue(data, dataSize, 0x80));
        break;
    };
    return code.str();
}

static std::string generateExpression2_Expr(const uint8_t *data, size_t dataSize, int numberOfGlobalConstants, int depth=0)
{
    ++depth;
    const unsigned int type = (depth > 3) ? 0 : getValue(data, dataSize, 3);
    const char binop[] = "=<>+-*/%&|^";
    const char *unop[] = {"++","--","()","~"};

    switch (type) {
    case 0:
        return generateExpression2_Op(data, dataSize, numberOfGlobalConstants);
    case 1: {
        const char op = binop[getValue(data,dataSize,sizeof(binop)-1)];
        const std::string lhs = (op == '=') ?
                                generateExpression2_lvalue(data, dataSize) :
                                generateExpression2_Expr(data, dataSize, numberOfGlobalConstants, depth);
        const std::string rhs = generateExpression2_Expr(data, dataSize, numberOfGlobalConstants, depth);

        return lhs + op + rhs;
    }
    case 2: {
        const char *u = unop[getValue(data,dataSize,sizeof(unop)/sizeof(*unop))];
        if (u == std::string("()"))
            return "(" + generateExpression2_Expr(data, dataSize, numberOfGlobalConstants, depth) + ")";
        else if (u == std::string("++") || u == std::string("--"))
            return u + generateExpression2_lvalue(data, dataSize);
        return u + generateExpression2_Expr(data, dataSize, numberOfGlobalConstants, depth);
    }
    default:
        break;
    };

    return "0";
}


static std::string generateExpression2_Cond(const uint8_t *data, size_t dataSize, int numberOfGlobalConstants)
{
    const char *comp[] = {"==", "!=", "<", "<=", ">", ">="};
    const int i = getValue(data, dataSize, 6);
    const std::string lhs = generateExpression2_Expr(data, dataSize, numberOfGlobalConstants);
    const std::string rhs = generateExpression2_Expr(data, dataSize, numberOfGlobalConstants);
    return lhs + comp[i] + rhs;
}


static std::string functionStart()
{
    static int functionNumber;
    return "int f" + std::to_string(++functionNumber) + "()\n"
           "{\n";
}

static std::string generateExpression2_conditionalCode(const std::string &indent,
        const uint8_t *data,
        size_t dataSize,
        int numberOfGlobalConstants)
{
    std::ostringstream code;

    if (indent.empty())
        code << functionStart();
    else
        code << indent << "{\n";

    for (int line = 0; line < 4 || indent.empty(); ++line) {
        bool done = false;
        const int type1 = getValue(data, dataSize, 8, &done);
        if (done)
            break;

        const int mostLikelyType = (line >= 2) ? 4 : 0;  // should var assignment or return be more likely?

        const int type2 = (indent.size() >= 12) ?
                          mostLikelyType :  // max indentation, no inner conditions
                          ((type1 >= 5) ? mostLikelyType : type1);

        if (type2 == 0) {
            code << indent << "    var" << getValue(data, dataSize, 5) << "=" << generateExpression2_Expr(data, dataSize, numberOfGlobalConstants) << ";\n";
        } else if (type2 == 1) {
            code << indent << "    if (" << generateExpression2_Cond(data, dataSize, numberOfGlobalConstants) << ")\n";
            code << generateExpression2_conditionalCode(indent + "    ", data, dataSize, numberOfGlobalConstants);
        } else if (type2 == 2) {
            code << indent << "    if (" << generateExpression2_Cond(data, dataSize, numberOfGlobalConstants) << ")\n";
            code << generateExpression2_conditionalCode(indent + "    ", data, dataSize, numberOfGlobalConstants);
            code << indent << "    else\n";
            code << generateExpression2_conditionalCode(indent + "    ", data, dataSize, numberOfGlobalConstants);
        } else if (type2 == 3) {
            code << indent << "    while (" << generateExpression2_Cond(data, dataSize, numberOfGlobalConstants) << ")\n";
            code << generateExpression2_conditionalCode(indent + "    ", data, dataSize, numberOfGlobalConstants);
        } else if (type2 == 4) {
            code << indent << "    return " << generateExpression2_Expr(data, dataSize, numberOfGlobalConstants) << ";\n";
            if (indent.empty())
                code << "}\n\n" << functionStart();
            else
                break;
        }
    }

    if (!indent.empty())
        code << indent << "}\n";
    else
        code << "    return 0;\n}\n";
    return code.str();
}

std::string generateCode2(const uint8_t *data, size_t dataSize)
{
    std::ostringstream code;

    // create global constants
    constexpr int numberOfGlobalConstants = 0;
    /*
      const int numberOfGlobalConstants = getValue(data, dataSize, 5);
      for (int nr = 1; nr <= numberOfGlobalConstants; nr++) {
        const char *types[4] = {"char", "int", "long long", "float"};
        code << "const " << types[getValue(data, dataSize, 4)] << " globalconstant" << nr << " = " << generateExpression2_Expr(data, dataSize, nr - 1) << ";\n";
      }
    */

    code << "int var1 = 1;\n"
         "int var2 = 0;\n"
         "int var3 = 1;\n"
         "int var4 = 0;\n"
         "int var5 = -1;\n\n";

    code << generateExpression2_conditionalCode("", data, dataSize, numberOfGlobalConstants);

    return code.str();
}


