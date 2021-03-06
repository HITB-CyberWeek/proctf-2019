package ae.hitb.proctf.drone_racing.programming.stack

import ae.hitb.proctf.drone_racing.programming.language.*

sealed class StackStatement

object Nop : StackStatement()
object Pop : StackStatement()
data class PushInt(val constant: IntLiteral) : StackStatement()
data class PushString(val constant: StringLiteral) : StackStatement()
data class PushPooled(val id: Int) : StackStatement()
data class Ld(val v: Variable) : StackStatement()
data class LdParam(var p: Param) : StackStatement()
data class St(val v: Variable) : StackStatement()
data class Unop(val kind: UnaryOperationKind) : StackStatement()
data class Binop(val kind: BinaryOperationKind) : StackStatement()
data class Jmp(val nextInstruction: Int) : StackStatement()
data class Jz(val nextInstruction: Int) : StackStatement()
data class Call(val function: FunctionDeclaration): StackStatement()
object TransEx : StackStatement()
object Ret1 : StackStatement()
object Ret0 : StackStatement()

data class StackProgram(val functions: Map<FunctionDeclaration, List<StackStatement>>,
                        val entryPoint: FunctionDeclaration,
                        val literalPool: List<CharArray>)

val currentExceptionVariable = Variable("___current_exception")
val poppedUnusedValueVariable = Variable("___popped_unused")
val returnDataVariable = Variable("___return_data")

