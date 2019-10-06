package ae.hitb.proctf.drone_racing.programming.language

sealed class Expression

data class Variable(val name: String) : Expression()

data class Param(val name: String): Expression()

data class IntLiteral(val value: Int) : Expression()
data class StringLiteral(val value: String) : Expression()

data class ArrayLiteral(val isBoxed: Boolean, val initializers: List<Expression>) : Expression()

data class FunctionCall(val functionDeclaration: FunctionDeclaration,
                        val argumentExpressions: List<Expression>) : Expression() {
    init { require(argumentExpressions.size == functionDeclaration.parameterNames.size) }
}

data class UnaryOperation(val operand: Expression, val kind: UnaryOperationKind) : Expression()
sealed class UnaryOperationKind
object Not : UnaryOperationKind()

fun UnaryOperationKind.semantics(x: Int) = when (this) {
    Not -> (x == 0).asBit()
}

data class BinaryOperation(val left: Expression, val right: Expression, val kind: BinaryOperationKind) : Expression()
sealed class BinaryOperationKind
object Plus : BinaryOperationKind()
object Minus : BinaryOperationKind()
object Times : BinaryOperationKind()
object Div : BinaryOperationKind()
object Rem : BinaryOperationKind()
object And : BinaryOperationKind()
object Or : BinaryOperationKind()
object Eq : BinaryOperationKind()
object Neq : BinaryOperationKind()
object Gt : BinaryOperationKind()
object Lt : BinaryOperationKind()
object Leq : BinaryOperationKind()
object Geq : BinaryOperationKind()

fun Boolean.asBit() = if (this) 1 else 0

fun BinaryOperationKind.semantics(l: Int, r: Int) = when (this) {
    Plus -> l + r
    Minus -> l - r
    Times -> l * r
    Div -> l / r
    Rem -> l % r
    And -> (l != 0 && r != 0).asBit()
    Or -> (l != 0 || r != 0).asBit()
    Eq -> (l == r).asBit()
    Neq -> (l != r).asBit()
    Gt -> (l > r).asBit()
    Lt -> (l < r).asBit()
    Leq -> (l <= r).asBit()
    Geq -> (l >= r).asBit()
}