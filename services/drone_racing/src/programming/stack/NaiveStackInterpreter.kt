package ae.hitb.proctf.drone_racing.programming.stack

import ae.hitb.proctf.drone_racing.programming.Interpreter
import ae.hitb.proctf.drone_racing.programming.exhaustive
import ae.hitb.proctf.drone_racing.programming.language.Intrinsic
import ae.hitb.proctf.drone_racing.programming.language.Variable
import ae.hitb.proctf.drone_racing.programming.language.andMap
import ae.hitb.proctf.drone_racing.programming.language.semantics

data class StackMachineState(val input: List<Int>,
                             val output: List<Int?>,
                             val variableValues: Map<Variable, Int>,
                             val stack: List<Int>,
                             val instructionPointer: Int,
                             val thrownExceptionId: Int,
                             val stringPool: List<CharArray>,
                             val arraysHeap: List<IntArray>)

class NaiveStackInterpreter() : Interpreter<StackMachineState, StackProgram, List<Int>> {
    override fun initialState(input: List<Int>): StackMachineState {
        val emptyState = emptyMap<Variable, Int>().andMap(currentExceptionVariable to 0)
        return StackMachineState(input, emptyList(), emptyState, emptyList(), 0, 0, emptyList(), emptyList())
    }

    private fun StackMachineState.pop(n: Int) = copy(stack = stack.dropLast(n))
    private fun StackMachineState.push(c: Int) = copy(stack = stack + c)
    private fun StackMachineState.addNewStr(str: CharArray) = copy(stack = stack + stringPool.size, stringPool = stringPool + str)

    fun step(s: StackMachineState, p: StackProgram) = with(s) {
        val code = p.functions[p.entryPoint]!!
        val t = code[s.instructionPointer]
        val step = when (t) {
            Nop -> this
            is Push -> copy(stack = stack + t.constant.value)
            is PushPooled -> copy(stack = stack + t.id)
            is Ld -> copy(stack = stack + variableValues[t.v]!!)
            is St -> copy(stack = stack.dropLast(1), variableValues = variableValues.andMap(t.v to stack.last()))
            is Unop ->
                copy(stack = stack.dropLast(1) + t.kind.semantics(stack.last()))
            is Binop -> {
                val l = stack[stack.lastIndex - 1]
                val r = stack.last()
                copy(stack = stack.dropLast(2) + t.kind.semantics(l, r))
            }
            is Jmp -> copy(instructionPointer = t.nextInstruction - 1)
            is Jz -> copy(stack = stack.dropLast(1),
                          instructionPointer = if (stack.last() != 0)
                                    instructionPointer else
                                    t.nextInstruction - 1)
            is Call -> t.function.let { f ->
                when (f) {
                    !is Intrinsic -> {
                        val internalMachine = StackMachineState(
                            input, output,
                            f.parameterNames.zip(stack.takeLast(f.parameterNames.size)).toMap()
                                .andMap(currentExceptionVariable to 0),
                            emptyList(), 0, 0, stringPool, arraysHeap)
                        val result = join(internalMachine, StackProgram(p.functions, f, p.literalPool))
                        val returnValue = result.stack.last()
                        val thrownException = result.thrownExceptionId
                        copy(result.input, result.output,
                             stack = stack.dropLast(f.parameterNames.size) + returnValue,
                             variableValues = variableValues.andMap(thrownExceptionVariable to thrownException),
                             stringPool = result.stringPool,
                             arraysHeap = result.arraysHeap)
                    }
                    else -> {
                        copy(variableValues = variableValues.andMap(thrownExceptionVariable to 0)).run {
                            when (f) {
                                Intrinsic.READ -> copy(input = input.drop(1), stack = stack + input.first(), output = output + null as Int?)
                                Intrinsic.WRITE, Intrinsic.WRITESTRING -> copy(output = output + stack.last(), stack = stack.dropLast(1).plus(0))
                                Intrinsic.STRMAKE -> {
                                    val (c, n) = stack.reversed()
                                    pop(2).addNewStr(CharArray(n) { c.toChar() })
                                }
                                Intrinsic.STRCMP -> {
                                    val (s2, s1) = stack.reversed()
                                    val cmpResult = String(stringPool[s1]).compareTo(String(stringPool[s2]))
                                    pop(2).push(cmpResult)
                                }
                                Intrinsic.STRGET -> {
                                    val (i, str) = stack.reversed()
                                    val result = stringPool[str][i].toInt()
                                    pop(2).push(result)
                                }
                                Intrinsic.STRDUP -> {
                                    val str = stack.last()
                                    pop(1).addNewStr(stringPool[str].copyOf())
                                }
                                Intrinsic.STRSET -> {
                                    val (c, i, str) = stack.reversed()
                                    stringPool[str][i] = c.toChar()
                                    pop(3).push(0)
                                }
                                Intrinsic.STRCAT -> {
                                    val (s2, s1) = stack.reversed()
                                    pop(2).addNewStr(stringPool[s1] + stringPool[s2])
                                }
                                Intrinsic.STRSUB -> {
                                    val (n, from, str) = stack.reversed()
                                    pop(3).addNewStr(stringPool[str].copyOfRange(from, from + n))
                                }
                                Intrinsic.STRLEN -> {
                                    val str = stack.last()
                                    pop(1).push(stringPool[str].size)
                                }
                                Intrinsic.ARRMAKE, Intrinsic.ARRMAKEBOX  -> {
                                    val (initVal, size) = stack.reversed()
                                    val newArray = IntArray(size) { initVal }
                                    val newArrayId = arraysHeap.size
                                    copy(arraysHeap = arraysHeap.plusElement(newArray)).pop(2).push(newArrayId)
                                }
                                Intrinsic.ARRGET -> {
                                    val (index, arrId) = stack.reversed()
                                    val result = arraysHeap[arrId][index]
                                    pop(2).push(result)
                                }
                                Intrinsic.ARRSET -> {
                                    val (value, index, arrid) = stack.reversed()
                                    val resultArray = arraysHeap[arrid].run { take(index) + value + drop(index + 1) }.toIntArray()
                                    val resultArrayHeap = arraysHeap.run { take(arrid).plusElement(resultArray) + drop(arrid + 1) }
                                    copy(arraysHeap = resultArrayHeap).pop(3).push(0)
                                }
                                Intrinsic.ARRLEN -> {
                                    val arrId = stack.last()
                                    val array = arraysHeap[arrId]
                                    pop(1).push(array.size)
                                }
                            }
                        }
                    }
                }.exhaustive
            }
            TransEx -> if (variableValues[currentExceptionVariable] == returnNormallyFakeExceptionId)
                this else
                copy (thrownExceptionId = variableValues[currentExceptionVariable]!!)
            Ret1 -> copy(instructionPointer = code.lastIndex)
            Ret0 -> copy(instructionPointer = code.lastIndex, stack = stack + 0)
            Pop -> copy(stack = stack.dropLast(1))
        }.exhaustive
        step.copy(instructionPointer = step.instructionPointer + 1)
    }

    override fun join(s: StackMachineState, p: StackProgram): StackMachineState =
            generateSequence(if (s.stringPool.isEmpty()) s.copy(stringPool = p.literalPool) else s) {
                if (it.instructionPointer !in p.code.indices) null else step(it, p)
            }.last()
}