package ae.hitb.proctf.drone_racing.programming.jvm

import ae.hitb.proctf.drone_racing.programming.Compiler
import ae.hitb.proctf.drone_racing.programming.exhaustive
import ae.hitb.proctf.drone_racing.programming.stack.*
import org.objectweb.asm.ClassWriter
import org.objectweb.asm.Label
import org.objectweb.asm.Opcodes.*
import org.objectweb.asm.Type.getDescriptor
import ae.hitb.proctf.drone_racing.programming.language.*
import java.io.BufferedReader


const val CLASS_NAME = "ae/hitb/proctf/drone_racing/DroneRacingProgram"
const val BASE_CLASS_NAME = "ae/hitb/proctf/drone_racing/StaticSharedMazeWalker"

class StackToJvmCompiler : Compiler<StackProgram, ByteArray> {
    private val brDescriptor = getDescriptor(BufferedReader::class.java)

    override fun compile(source: StackProgram): ByteArray {
        val cw = ClassWriter(ClassWriter.COMPUTE_FRAMES)
        cw.visit(V1_8, ACC_PUBLIC + ACC_SUPER, CLASS_NAME, null, BASE_CLASS_NAME, emptyArray())

        cw.visitMethod(ACC_PUBLIC, "<init>", "()V", null, null).apply {
            visitVarInsn(ALOAD, 0)
            visitMethodInsn(INVOKESPECIAL, BASE_CLASS_NAME, "<init>", "()V", false)
            visitInsn(RETURN)
            visitMaxs(-1, -1)
            visitEnd()
        }

        source.functions.forEach { (declaration, code) -> compileFunction(declaration, code, cw, source.literalPool) }

        return cw.toByteArray()
    }

    private fun getJavaType(functionType: FunctionType) : String {
        return when (functionType) {
            FunctionType.INTEGER -> "I"
            FunctionType.STRING -> "Ljava/lang/String;"
            FunctionType.VOID -> "V"
        }
    }

    private fun findFunctionVariableType(function: FunctionDeclaration, v: Variable): FunctionType {
        if (v == returnDataVariable && (function.returnType in listOf(FunctionType.STRING, FunctionType.INTEGER)))
            return function.returnType
        val index = function.parameterNames.indexOf(v)
        if (index < 0)
            return FunctionType.INTEGER
        return function.parameterTypes[index]
    }

    private fun compileFunction(
        function: FunctionDeclaration,
        source: List<StackStatement>,
        cw: ClassWriter,
        literalPool: List<CharArray>
    ) {
        val signature = "(" + function.parameterTypes.joinToString { getJavaType(it) } + ")" + getJavaType(function.returnType)

        cw.visitMethod(ACC_PUBLIC + ACC_STATIC, function.name, signature, null, null).apply {
            val beginLabel = Label().apply { info = "begin" }
            val endLabel = Label().apply { info = "end" }
            visitLabel(beginLabel)

            val variablesMap = function.parameterNames.withIndex().associate { (index, it) -> it to index }.toMutableMap()

            val functionVariables = (collectVariables(source) + function.parameterNames).distinct()
            val functionLocalVariables = functionVariables - function.parameterNames
            var variableIndex = function.parameterNames.size
            functionLocalVariables.forEach {
                variablesMap[it] = variableIndex++
            }
            variablesMap.forEach { (v, index) -> visitLocalVariable(v.name, getJavaType(findFunctionVariableType(function, v)), null, beginLabel, endLabel, index) }

            val labels = (source + NOP).map { Label().apply { info = it; } }

            for ((index, s) in source.withIndex()) {
                visitLabel(labels[index])

                when (s) {
                    Nop -> visitInsn(NOP)

                    is PushInt -> visitLdcInsn(s.constant.value)
                    is PushString -> visitLdcInsn(s.constant.value)
                    is Ld -> {
                        if (s.v == returnDataVariable && (function.returnType == FunctionType.INTEGER || function.returnType == FunctionType.VOID) || s.v != returnDataVariable)
                            visitVarInsn(ILOAD, variablesMap[s.v]!!)
                        else
                            visitVarInsn(ALOAD, variablesMap[s.v]!!)
                    }
                    is LdParam -> {
                        visitLdcInsn(s.p.name)
                        visitMethodInsn(INVOKESTATIC, "java/lang/System", "getProperty", "(Ljava/lang/String;)Ljava/lang/String;", false)
                    }
                    is St -> {
                        if (s.v == returnDataVariable && (function.returnType == FunctionType.INTEGER || function.returnType == FunctionType.VOID) || s.v != returnDataVariable)
                            visitVarInsn(ISTORE, variablesMap[s.v]!!)
                        else
                            visitVarInsn(ASTORE, variablesMap[s.v]!!)
                    }
                    is Unop -> when (s.kind) {
                        Not -> {
                            val labelIfNz = Label()
                            val labelAfter = Label()
                            visitJumpInsn(IFNE, labelIfNz)
                            visitInsn(ICONST_0)
                            visitJumpInsn(GOTO, labelAfter)
                            visitLabel(labelIfNz)
                            visitInsn(ICONST_1)
                            visitLabel(labelAfter)
                        }
                    }
                    is Binop -> when (s.kind) {
                        Plus -> visitInsn(IADD)
                        Minus -> visitInsn(ISUB)
                        Times -> visitInsn(IMUL)
                        Div -> visitInsn(IDIV)
                        Rem -> visitInsn(IREM)
                        And -> visitInsn(IAND)
                        Or -> visitInsn(IOR)
                        Eq, Neq, Gt, Lt, Leq, Geq -> {
                            val labelOtherwise = Label()
                            val labelAfter = Label()
                            visitJumpInsn(checkOtherwiseOp[s.kind]!!, labelOtherwise)
                            visitInsn(ICONST_1)
                            visitJumpInsn(GOTO, labelAfter)
                            visitLabel(labelOtherwise)
                            visitInsn(ICONST_0)
                            visitLabel(labelAfter)
                        }
                    }.exhaustive
                    is Jmp -> visitJumpInsn(GOTO, labels[s.nextInstruction])
                    is Jz -> {
                        visitInsn(ICONST_0)
                        visitJumpInsn(IF_ICMPEQ, labels[s.nextInstruction])
                    }
                    is Call -> when (s.function) {
                        is Intrinsic -> when (s.function) {
                            Intrinsic.TO_STRING -> {
                                visitMethodInsn(INVOKESTATIC, "java/lang/String", "valueOf", "(I)Ljava/lang/String;", false)
                            }
                            Intrinsic.STRMAKE -> {
                                visitMethodInsn(INVOKESTATIC, "java/lang/String", "valueOf", "(C)Ljava/lang/String;", false)
                                visitMethodInsn(INVOKESTATIC, "java/util/Collections", "nCopies", "(ILjava/lang/Object;)Ljava/util/List;", false)
                                visitLdcInsn("")
                                visitInsn(SWAP)
                                visitMethodInsn(INVOKESTATIC, "java/lang/String", "join", "(Ljava/lang/CharSequence;Ljava/lang/Iterable;)Ljava/lang/String;", false)
                            }
                            Intrinsic.STRCMP -> {
                                visitMethodInsn(INVOKEVIRTUAL, "java/lang/String", "compareTo","(Ljava/lang/String;)I", false);
                            }
                            Intrinsic.STRGET -> {
                                visitMethodInsn(INVOKEVIRTUAL, "java/lang/String", "charAt", "(I)C", false)
                            }
                            Intrinsic.STRDUP -> {
                                // NOP
                            }
                            Intrinsic.STRCAT -> {
                                visitInsn(SWAP)
                                visitTypeInsn(NEW, "java/lang/StringBuilder")
                                visitInsn(DUP)
                                visitMethodInsn(INVOKESPECIAL, "java/lang/StringBuilder", "<init>", "()V", false);
                                visitInsn(SWAP)
                                visitMethodInsn(INVOKEVIRTUAL, "java/lang/StringBuilder", "append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;", false)
                                visitInsn(SWAP)
                                visitMethodInsn(INVOKEVIRTUAL, "java/lang/StringBuilder", "append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;", false)
                                visitMethodInsn(INVOKEVIRTUAL, "java/lang/StringBuilder", "toString", "()Ljava/lang/String;", false)
                            }
                            Intrinsic.STRSUB -> {
                                visitMethodInsn(INVOKEVIRTUAL, "java/lang/String", "substring", "(II)Ljava/lang/String;", false)
                            }
                            Intrinsic.STRLEN -> visitMethodInsn(INVOKEVIRTUAL, "java/lang/String", "length", "()I", false)
                        }
                        else -> {
                            val descriptor = "(" + s.function.parameterTypes.joinToString { getJavaType(it) } + ")" + getJavaType(s.function.returnType)
                            visitMethodInsn(INVOKESTATIC, CLASS_NAME, s.function.name, descriptor, false)
                        }
                    }
                    Ret0 -> {
                        visitInsn(RETURN)
                    }
                    Ret1 -> {
                        if (function.returnType == FunctionType.STRING)
                            visitInsn(ARETURN)
                        else
                            visitInsn(IRETURN)
                    }
                    Pop -> visitInsn(POP)
                    is PushPooled -> {
                        visitLdcInsn(String(literalPool[s.id]))
                    }
                    TransEx -> visitInsn(NOP)
                }.exhaustive
            }

            visitLabel(labels.last())
            visitMaxs(0, 0)
            visitEnd()
        }
    }

    private val checkOtherwiseOp = mapOf(
        Eq to IF_ICMPNE,
        Neq to IF_ICMPEQ,
        Gt to IF_ICMPLE,
        Lt to IF_ICMPGE,
        Geq to IF_ICMPLT,
        Leq to IF_ICMPGT
    )
}