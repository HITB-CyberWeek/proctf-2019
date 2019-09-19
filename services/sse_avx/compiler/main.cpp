#include <string.h>
#include "frontend.h"

// V0, V1, ..., V15
#define V0_LOCATION "[rsp + 0]"
// TMP
#define TMP_LOCATION "[rsp + 512]"
// padding, TMP
//#define TMP_LOCATION "[rsp + 608]"


enum EVecType
{
    kAVX = 0,
    kSSE
};


struct XMM : public Register
{
    XMM(const Register& r)
    {
        type = r.type;
        idx = r.idx;
    }
};


struct YMM : public Register
{
    YMM(const Register& r)
    {
        type = r.type;
        idx = r.idx;
    }
};


void EmitComment(std::string& str, const Instruction& instr)
{
    char buf[256];

    auto emitOperand = [&](Instruction::Operand op)
    {
        if(op.type == kOperandImmediate)
            sprintf(buf, "0x%llx", op.imm);
        else if(op.type == kOperandRegister && op.reg.type == Register::kVector)
        {
            if(op.reg.rangeLen == 0)
                sprintf(buf, "v%u", op.reg.idx);
            else
                sprintf(buf, "v[%u:%u]", op.reg.idx, op.reg.idx + op.reg.rangeLen);
        }
        else if(op.type == kOperandRegister && op.reg.type == Register::kScalar)
            sprintf(buf, "s%u", op.reg.idx);
        else if(op.type == kOperandRegister && op.reg.type == Register::kScalar64)
            sprintf(buf, "s%uq", op.reg.idx);
        else if(op.type == kOperandRegister && op.reg.type == Register::kEXEC)
            sprintf(buf, "exec");
        else if(op.type == kOperandRegister && op.reg.type == Register::kVCC)
            sprintf(buf, "vcc");
        else if(op.type == kOperandLabelId)
            sprintf(buf, "%s", op.labelId);
        
        str.append(buf);
    };

    if(!instr.label.empty())
    {
        sprintf(buf, "    ; %s:\n", instr.label.c_str());
        str.append(buf);
    }

    str.append("    ; ");
    str.append(kOpToMnemonic[instr.opCode]);
    str.append(" ");
    uint32_t opsNum = kOpOperandsNum[instr.opCode];
    for(uint32_t i = 0; i < opsNum; i++)
    {
        emitOperand(instr.operands[i]);
        if (i != opsNum - 1)
            str.append(", ");
    }
    str.append("\n");
}


void Fmt(std::string& str, const char* formatString)
{
    str.append(formatString);
};


template<typename... Args>
void Fmt(std::string& str, const char* formatString, Args... args)
{
    char buf[512];
    sprintf(buf, formatString, args...);
    str.append(buf);
};


const char* EmitImmediate(char* buf, uint64_t imm)
{
    sprintf(buf, "0x%llx", imm);
    return buf;
}


template<EVecType type>
const char* EmitRegister(char* buf, const Register& reg)
{
    if(reg.type == Register::kVector)
        sprintf(buf, type == kAVX ? "ymm%u" : "xmm%u", reg.idx);
    else if(reg.type == Register::kScalar)
        sprintf(buf, "r%ud", 8 + reg.idx);
    else if(reg.type == Register::kScalar64)
        sprintf(buf, "r%u", 8 + reg.idx);
    else if(reg.type == Register::kVCC)
        sprintf(buf, "ebx");
    else if(reg.type == Register::kEXEC)
        sprintf(buf, "ecx");
    return buf;
}


template<EVecType type>
const char* EmitOperand(char* buf, const Instruction::Operand& op)
{
    if(op.type == kOperandImmediate)
        return EmitImmediate(buf, op.imm);
    else if(op.type == kOperandRegister)
        return EmitRegister<type>(buf, op.reg);
    return nullptr;
}


template<EVecType type>
std::string& operator<<(std::string& str, const char* cstr)
{
    str.append(cstr);
    return str;
}


template<EVecType type>
std::string& operator<<(std::string& str, uint64_t imm)
{
    char buf[32];
    str.append(EmitImmediate(buf, imm));
    return str;
}


template<EVecType type>
std::string& operator<<(std::string& str, const Register& reg)
{
    char buf[16];
    str.append(EmitRegister<type>(buf, reg));
    return str;
}


template<EVecType type>
std::string& operator<<(std::string& str, const XMM& xmm)
{
    char buf[16];
    str.append(EmitRegister<kSSE>(buf, xmm));
    return str;
}


template<EVecType type>
std::string& operator<<(std::string& str, const YMM& ymm)
{
    char buf[16];
    str.append(EmitRegister<kAVX>(buf, ymm));
    return str;
}


template<EVecType type>
std::string& operator<<(std::string& str, const Instruction::Operand& op)
{
    char buf[16];
    str.append(EmitOperand<type>(buf, op));
    return str;
}


template<EVecType type>
void AddLine(std::string& str, uint32_t argsNum, uint32_t& argsCounter)
{
    str.append("\n");
}


template<EVecType type, typename T, typename... Args>
void AddLine(std::string& str, uint32_t argsNum, uint32_t& argsCounter, T t, const Args&... args)
{
    operator<<<type>(str, t);
    if(argsCounter == 0 && argsNum > 1)
        str.append(" ");
    else if(argsCounter < argsNum - 1)
        str.append(", ");
    argsCounter++;
    AddLine<type>(str, argsNum, argsCounter, args...);
}


template<EVecType type, bool noTab = false, typename... Args>
void AddLine(std::string& str, const Args&... args)
{
    uint32_t argsNum = sizeof...(args);
    uint32_t argsCounter = 0;
    if(!noTab)
        str.append("    ");
    AddLine<type>(str, argsNum, argsCounter, args...);
}


template<typename... Args>
void AddLine(std::string& str, const char* formatString, Args... args)
{
    Fmt(str, formatString, args...);
    str.append("\n");
};


void EmitStoreLoad(std::string& asmbl, uint32_t usedVgprRegistersMask)
{
    AddLine(asmbl, "store_load_avx:");
    if(usedVgprRegistersMask)
    {
        AddLine(asmbl, "    and ecx, 0xff");
        AddLine(asmbl, "    mov eax, edx");
        AddLine(asmbl, "    xor eax, ecx");
        AddLine(asmbl, "    jz save_exec_avx");
        AddLine(asmbl, "    vmovd xmm15, edx");
        AddLine(asmbl, "    vshufps ymm15, ymm15, ymm15, 0");
        AddLine(asmbl, "    vperm2f128 ymm15, ymm15, ymm15, 0");
        AddLine(asmbl, "    vpand ymm15, ymm15, [rdi]");
        AddLine(asmbl, "    vpcmpeqd ymm15, ymm15, [rdi]");
        uint32_t tmpMask = usedVgprRegistersMask;
        while(tmpMask)
        {
            uint32_t idx = __builtin_ctz(tmpMask);
            tmpMask &= ~(1 << idx);
            char buf[128];
            sprintf(buf, "    vmaskmovps [rsp + %u], ymm15, ymm%u\n", idx * 32, idx);
            asmbl.append(buf);
            sprintf(buf, "    vmovaps ymm%u, [rsp + %u]\n", idx, idx * 32);
            asmbl.append(buf);
        }
        AddLine(asmbl, "save_exec_avx:");
    }

    AddLine(asmbl, "    mov edx, ecx");
    AddLine(asmbl, "    jmp rbp");
}


template<EVecType type>
void PostScalarInstruction(std::string& asmbl, const Instruction& i)
{
    if(kOpToType[i.opCode] != kInstructionTypeScalar || kOpOperandsNum[i.opCode] == 0)
        return;

    const Instruction::Operand& op0 = i.operands[0];
    if(op0.type != kOperandRegister || op0.reg.type != Register::kEXEC)
        return;

    static uint32_t postScalarCounter = 0;
    const char* postfix = type == kAVX ? "avx" : "sse";

    AddLine(asmbl, "post_scalar_%s_%u_0:", postfix, postScalarCounter);
    AddLine(asmbl, "    lea rbp, [rel $]");
    AddLine(asmbl, "    add rbp, post_scalar_%s_%u_1 - post_scalar_%s_%u_0", postfix, postScalarCounter, postfix, postScalarCounter);
    AddLine(asmbl, "    jmp store_load_avx");
    AddLine(asmbl, "post_scalar_%s_%u_1:", postfix, postScalarCounter);

    AddLine(asmbl, "    mov eax, ecx");
    AddLine(asmbl, "    and eax, 0xf0");
    if(type == kAVX)
        AddLine(asmbl, "    jz post_scalar_sse_%u_2", postScalarCounter);
    else
        AddLine(asmbl, "    jnz post_scalar_avx_%u_2", postScalarCounter);
    AddLine(asmbl, "post_scalar_%s_%u_2:", postfix, postScalarCounter);

    postScalarCounter++;
}


template<EVecType type>
void EmitBroadcast(std::string& asmbl, const Register& dst, const Instruction::Operand& src)
{
    bool srcImm = src.type == kOperandImmediate;
    if(srcImm)
        AddLine<type>(asmbl, "mov", "eax", src.imm);

    if(type == kAVX)
    {
        if(srcImm)
            AddLine<kAVX>(asmbl, "vmovd", XMM(dst), "eax");
        else
            AddLine<kAVX>(asmbl, "vmovd", XMM(dst), src);
        AddLine<kAVX>(asmbl, "vshufps", dst, dst, dst, 0);
        AddLine<kAVX>(asmbl, "vperm2f128", dst, dst, dst, 0);
    }
    else
    {
        if(srcImm)
            AddLine<kAVX>(asmbl, "movd", XMM(dst), "eax");
        else
            AddLine<kAVX>(asmbl, "movd", XMM(dst), src);
        AddLine<kSSE>(asmbl, "shufps", dst, dst, 0);
    }
}


template<EVecType type>
void GenerateCode(std::string& asmbl, const ParsedCode& parsedCode)
{
    AddLine(asmbl, "start_%s:", type == kAVX ? "avx" : "sse");

    for(auto& inst : parsedCode.instructions)
    {
        EmitComment(asmbl, inst);

        if(!inst.label.empty())
            AddLine(asmbl, "%s_%s:", inst.label.c_str(), type == kAVX ? "avx" : "sse");

        switch (inst.opCode)
        {
            case kVectorMov:
            {
                auto& dstReg = inst.operands[0].reg;
                auto& srcOp = inst.operands[1];
                if(srcOp.type == kOperandRegister && srcOp.reg.type == Register::kVector)
                {
                    const char* mnemonic = type == kAVX ? "vmovaps" : "movaps";
                    AddLine<type>(asmbl, mnemonic, inst.operands[0], inst.operands[1]);
                }
                else
                {
                    EmitBroadcast<type>(asmbl, dstReg, srcOp);
                }
                break;
            }

            case kVectorAdd_f32:
            case kVectorSub_f32:
            case kVectorMul_f32:
            case kVectorDiv_f32:
            case kVectorAdd_u32:
            case kVectorSub_u32:
            case kVectorMul_u32:
            {
                const char* mnemonic = nullptr;
                if(inst.opCode == kVectorAdd_f32)
                    mnemonic = type == kAVX ? "vaddps" : "addps";
                else if(inst.opCode == kVectorSub_f32)
                    mnemonic = type == kAVX ? "vsubps" : "subps";
                else if(inst.opCode == kVectorMul_f32)
                    mnemonic = type == kAVX ? "vmulps" : "mulps";
                else if(inst.opCode == kVectorDiv_f32)
                    mnemonic = type == kAVX ? "vdivps" : "divps";
                else if(inst.opCode == kVectorAdd_u32)
                    mnemonic = type == kAVX ? "vpaddd" : "paddd";
                else if(inst.opCode == kVectorSub_u32)
                    mnemonic = type == kAVX ? "vpsubd" : "psubd";
                else if(inst.opCode == kVectorMul_u32)
                    mnemonic = type == kAVX ? "vpmulld" : "pmulld";
                
                Register src2;
                if(inst.operands[2].type == kOperandImmediate)
                {
                    src2 = Register(Register::kVector, 15);
                    EmitBroadcast<type>(asmbl, src2, inst.operands[2]);
                }
                else
                {
                    src2 = inst.operands[2].reg;
                }

                if(type == kAVX)
                {
                    AddLine<kAVX>(asmbl, mnemonic, inst.operands[0], inst.operands[1], src2);
                }
                else
                {
                    if(inst.operands[0].reg != inst.operands[1].reg)
                        AddLine<kSSE>(asmbl, "movaps", inst.operands[0], inst.operands[1]);
                    AddLine<kSSE>(asmbl, mnemonic, inst.operands[0], src2);
                }
                break;
            }

            case kVectorCmpEq_f32:
            case kVectorCmpGt_f32:
            case kVectorCmpEq_u32:
            {
                const char* mnemonic = nullptr;
                if(inst.opCode == kVectorCmpEq_f32)
                    mnemonic = type == kAVX ? "vcmpeqps" : "cmpeqps";
                else if(inst.opCode == kVectorCmpGt_f32)
                    mnemonic = type == kAVX ? "vcmpgtps" : "cmpnleps";
                else if(inst.opCode == kVectorCmpEq_u32)
                    mnemonic = type == kAVX ? "vpcmpeqd" : "pcmpeqd";

                auto& reg0 = inst.operands[0].reg;
                auto& reg1 = inst.operands[1].reg;
                if(type == kAVX)
                {
                    AddLine<type>(asmbl, mnemonic, "ymm15", reg0, reg1);
                    AddLine<type>(asmbl, "vmovmskps ebx, ymm15");
                }
                else
                {
                    AddLine<type>(asmbl, "movaps", "xmm15", reg0);
                    AddLine<type>(asmbl, mnemonic, "xmm15", reg1);
                    AddLine<type>(asmbl, "movmskps ebx, xmm15");
                }
                AddLine<type>(asmbl, "and ebx, ecx");
                break;
            }

            case kVectorLoad:
            {
                auto dstReg = inst.operands[0].reg;
                auto& offsetOp = inst.operands[1];
                auto& srcReg = inst.operands[2].reg;

                if(offsetOp.type == kOperandRegister && offsetOp.reg.type == Register::kVector)
                {
                    auto& offsetReg = inst.operands[1].reg;
                    for(uint32_t v = 0; v <= dstReg.rangeLen; v++, dstReg.idx++)
                    {
                        /*const uint32_t kHalfsNum = type == kAVX ? 2 : 1;
                        for(uint32_t halfIdx = 0; halfIdx < kHalfsNum; halfIdx++)
                        {
                            AddLine<kAVX>(asmbl, "vextractf128", "xmm15", offsetReg, halfIdx);
                            for(uint32_t i = 0; i < 4; i++)
                            {
                                AddLine<kAVX>(asmbl, "vpextrd", "eax", "xmm15", i);
                                AddLine<type>(asmbl, "shl", "eax", 2);
                                AddLine<type>(asmbl, "add", "rax", srcReg);
                                AddLine<type>(asmbl, "mov eax, [rax]");
                                AddLine(asmbl, "    mov [rsp + 512 + %u * 4], eax", i);
                            }
                        }
                        AddLine<kAVX>(asmbl, "vmovaps", dstReg, "[rsp + 512]");*/
                        Register temp(Register::kVector, 15);
                        AddLine<type>(asmbl, "vmovd", XMM(temp), "ecx");
                        AddLine<type>(asmbl, "vshufps", temp, temp, temp, "0");
                        if(type == kAVX)
                            AddLine<type>(asmbl, "vperm2f128", temp, temp, temp, "0");
                        AddLine<type>(asmbl, "vpand", temp, temp, "[rdi]");
                        AddLine<type>(asmbl, "vpcmpeqd", temp, temp, "[rdi]");
                        
                        asmbl.append("    vpgatherdd ");
                        operator<<<type>(asmbl, dstReg);
                        asmbl.append(", [");
                        operator<<<type>(asmbl, offsetReg);
                        asmbl.append(" * 4 + ");
                        operator<<<type>(asmbl, srcReg);
                        Fmt(asmbl, " + %u * 4], ", v);
                        operator<<<type>(asmbl, temp);
                        asmbl.append("\n");
                    }
                }
                else
                {
                    for(uint32_t v = 0; v <= dstReg.rangeLen; v++, dstReg.idx++)
                    {
                        char buf[128], buf1[32];
                        asmbl.append("    ");
                        asmbl.append(type == kAVX ? "vmovaps " : "movaps ");
                        asmbl.append(EmitRegister<type>(buf1, dstReg));
                        uint32_t offset = v * 32;
                        if(offsetOp.type == kOperandImmediate)
                            offset += offsetOp.imm * 4;
                        sprintf(buf, ", [%s + %u]\n", EmitRegister<type>(buf1, srcReg), offset);
                        asmbl.append(buf);
                    }
                }
                break;
            }

            case kVectorStore:
            {
                auto srcReg = inst.operands[0].reg;
                auto& offsetOp = inst.operands[1];
                auto& dstReg = inst.operands[2].reg;

                if(offsetOp.type == kOperandRegister && offsetOp.reg.type == Register::kVector)
                {
                    auto& offsetReg = inst.operands[1].reg;
                    for(uint32_t v = 0; v <= srcReg.rangeLen; v++, srcReg.idx++)
                    {
                        const uint32_t kHalfsNum = type == kAVX ? 2 : 1;
                        for(uint32_t halfIdx = 0; halfIdx < kHalfsNum; halfIdx++)
                        {
                            for(uint32_t i = 0; i < 4; i++)
                            {
                                uint32_t laneIdx = halfIdx * 4 + i;
                                if(type == kAVX)
                                {
                                    AddLine<kAVX>(asmbl, "vextractf128", "xmm15", offsetReg, halfIdx);
                                    AddLine<kAVX>(asmbl, "vpextrd", "rbp", "xmm15", i);
                                }
                                else
                                    AddLine<kSSE>(asmbl, "pextrd", "ebp", offsetReg, i);
                                
                                AddLine<type>(asmbl, "shl", "ebp", 2);
                                AddLine<type>(asmbl, "add", "rbp", dstReg);
                                if(v != 0)
                                    AddLine(asmbl, "    add rbp, %u", v * 4);
                                AddLine(asmbl, "    mov eax, %u", 1 << laneIdx);
                                AddLine(asmbl, "    and eax, ecx");
                                AddLine(asmbl, "    cmovz rbp, rsi");
                                
                                if(type == kAVX)
                                {
                                    AddLine<kAVX>(asmbl, "vextractf128", "xmm15", srcReg, halfIdx);
                                    AddLine<kAVX>(asmbl, "vextractps", "[rbp]", "xmm15", i);
                                }
                                else
                                {
                                    AddLine<kSSE>(asmbl, "extractps", "[rbp]", srcReg, i);
                                }
                                
                            }
                        }
                    }
                }
                else
                {
                    AddLine(asmbl, "    vmovd xmm15, ecx");
                    AddLine(asmbl, "    vshufps ymm15, ymm15, ymm15, 0");
                    AddLine(asmbl, "    vperm2f128 ymm15, ymm15, ymm15, 0");
                    AddLine(asmbl, "    vpand ymm15, ymm15, [rdi]");
                    AddLine(asmbl, "    vpcmpeqd ymm15, ymm15, [rdi]");

                    for(uint32_t v = 0; v <= srcReg.rangeLen; v++, srcReg.idx++)
                    {
                        char buf[128], buf1[32];
                        asmbl.append("    ");
                        asmbl.append("vmaskmovps ");
                        uint32_t offset = v * 32;
                        if(offsetOp.type == kOperandImmediate)
                            offset += offsetOp.imm * 4;
                        sprintf(buf, "[%s + %u], ymm15, ", EmitRegister<type>(buf1, dstReg), offset);
                        asmbl.append(buf);
                        asmbl.append(EmitRegister<kAVX>(buf1, srcReg));
                        asmbl.append("\n");
                    }
                }
                break;
            }

            case kScalarMov:
                AddLine<type>(asmbl, "mov", inst.operands[0], inst.operands[1]);
                break;

            case kScalarAddu:
            case kScalarAnd:
            {
                const char* mnemonic = nullptr;
                if(inst.opCode == kScalarAddu)
                    mnemonic = "add";
                else if(inst.opCode == kScalarAnd)
                    mnemonic = "and";

                auto& dst = inst.operands[0];
                auto& src0 = inst.operands[1];
                auto& src1 = inst.operands[2];
                if(src1.type == kOperandRegister && dst.reg == src1.reg)
                {
                    AddLine<type>(asmbl, mnemonic, dst, src0);
                }
                else if(src0.type == kOperandRegister && dst.reg == src0.reg)
                {
                    AddLine<type>(asmbl, mnemonic, dst, src1);
                }
                else
                {
                    AddLine<type>(asmbl, "mov", dst, src0);
                    AddLine<type>(asmbl, mnemonic, dst, src1);
                }
                break;
            }

            case kScalarAndN2:
            {
                auto& dst = inst.operands[0];
                auto& src0 = inst.operands[1];
                auto& src1 = inst.operands[2];
                const char* tmpReg = dst.reg.type == Register::kScalar64 ? "rax" : "eax";
                if(src1.type == kOperandRegister && dst.reg == src1.reg)
                {
                    AddLine<type>(asmbl, "not", dst);
                    AddLine<type>(asmbl, "and", dst, src0);
                }
                else if(src0.type == kOperandRegister && dst.reg == src0.reg)
                {
                    AddLine<type>(asmbl, "mov", tmpReg, src1);
                    AddLine<type>(asmbl, "not", tmpReg);
                    AddLine<type>(asmbl, "and", dst, tmpReg);
                }
                else
                {
                    AddLine<type>(asmbl, "mov", tmpReg, src1);
                    AddLine<type>(asmbl, "not", tmpReg);
                    AddLine<type>(asmbl, "mov", dst, src0);
                    AddLine<type>(asmbl, "and", dst, tmpReg);
                }
                break;
            }
            
            default:
                break;
        }

        PostScalarInstruction<type>(asmbl, inst);
    }

    AddLine<type>(asmbl, "jmp exit");
}


void* ReadFile(const char* fileName, uint32_t& size)
{
    FILE* f = fopen(fileName, "r");
    if (!f)
    {
        size = 0;
        return nullptr;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    void* fileData = malloc(size);
    fread(fileData, 1, size, f);
    fclose(f);

    return fileData;
}


bool WriteFile(const char* fileName, const void* data, uint32_t size)
{
    FILE* f = fopen(fileName, "w");
    if(!f)
        return false;
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    return written == size;
}


bool Compile(const std::string& asmbl, uint32_t groupDimX, uint32_t groupDimY, const char* outputFile)
{
    const char* nasmInputFile = "/tmp/nasm_input";
    if(!WriteFile(nasmInputFile, asmbl.c_str(), asmbl.length()))
    {
        printf("Failed to write file %s\n", nasmInputFile);
        return false;
    }

    const char* nasmOutputFile = "/tmp/nasm_output";
    char cmd[512];
    sprintf(cmd, "nasm -f elf64 %s -o %s", nasmInputFile, nasmOutputFile);
    if(system(cmd) != 0)
    {
        printf("nasm failed\n");
        return false;
    }

    const char* ldOutputFile = "/tmp/ld_output";
    sprintf(cmd, "ld %s -N -Ttext 0 -o %s", nasmOutputFile, ldOutputFile);
    if(system(cmd) != 0)
    {
        printf("ld failed\n");
        return false;
    }

    const char* objcopyOutputFile = "/tmp/objcopy_output";
    sprintf(cmd, "objcopy -O binary %s %s", ldOutputFile, objcopyOutputFile);
    if(system(cmd) != 0)
    {
        printf("objcopy failed\n");
        return false;
    }

    uint32_t binarySize = 0;
    void* binary = ReadFile(objcopyOutputFile, binarySize);
    if(!binary)
    {
        printf("Failed to read file %s\n", objcopyOutputFile);
        return false;
    }

    uint32_t patchedBinarySize = binarySize + 2 * sizeof(uint32_t);
    void* patchedBinary = malloc(patchedBinarySize);
    uint8_t* ptr = (uint8_t*)patchedBinary;
    memcpy(ptr, &groupDimX, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(ptr, &groupDimY, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(ptr, binary, binarySize);

    if(!WriteFile(outputFile, patchedBinary, patchedBinarySize))
    {
        printf("Failed to write file %s\n", outputFile);
        return false;
    }

    free(binary);
    free(patchedBinary);

    return true;
}


void GetUsedRegistersMask(const ParsedCode& parsedCode, uint32_t& usedSgprRegistersMask, uint32_t& usedVgprRegistersMask)
{
    usedSgprRegistersMask = 0;
    usedVgprRegistersMask = 0;
    for(uint32_t i = 0; i < parsedCode.instructions.size(); i++)
    {
        auto& inst = parsedCode.instructions[i];
        uint32_t opsNum = kOpOperandsNum[inst.opCode];
        for(uint32_t j = 0; j < opsNum; j++)
        {
            auto& op = inst.operands[j];
            if(op.type != kOperandRegister)
                continue;
            
            if(op.reg.type == Register::kScalar || op.reg.type == Register::kScalar64)
                usedSgprRegistersMask |= 1 << op.reg.idx;
            else if(op.reg.type == Register::kVector)
                usedVgprRegistersMask |= 1 << op.reg.idx;
        }
    }
}


int main(int argc, const char* argv[]) 
{
    if(argc != 3)
    {
        printf("compiler <input> <output>\n");
        return -1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];

    ParsedCode parsedCode;
    if(!Parse(inputFile, parsedCode))
        return -1;

    const uint32_t kMaxGroupDim = 16;
    if(parsedCode.groupDimX == 0 || parsedCode.groupDimX > kMaxGroupDim || 
       parsedCode.groupDimY == 0 || parsedCode.groupDimY > kMaxGroupDim)
    {
        printf("Invalid group dimension\n");
        return -1;
    }

    uint32_t usedSgprRegistersMask = 0, usedVgprRegistersMask = 0;
    GetUsedRegistersMask(parsedCode, usedSgprRegistersMask, usedVgprRegistersMask);

    std::vector<uint32_t> regsToSave;
    uint32_t tmpMask = usedSgprRegistersMask;
    while(tmpMask)
    {
        uint32_t idx = __builtin_ctz(tmpMask);
        tmpMask &= ~(1 << idx);
        uint32_t x86RegIdx = 8 + idx;
        if(x86RegIdx >= 12)
            regsToSave.push_back(x86RegIdx);
    }

    std::string asmbl;    
    AddLine(asmbl, "[SECTION .text]");
    AddLine(asmbl, "global _start");
    AddLine(asmbl, "_start:");
    AddLine(asmbl, "    push rbp");
    AddLine(asmbl, "    push rbx");
    for(auto iter = regsToSave.begin(); iter != regsToSave.end(); ++iter)
        AddLine(asmbl, "    push r%u", *iter);
    AddLine(asmbl, "    mov rbp, rsp");
    AddLine(asmbl, "    and rsp, 0xffffffffffffffe0");
    AddLine(asmbl, "    sub rsp, 32");
    AddLine(asmbl, "    mov [rsp], rbp");
    AddLine(asmbl, "    sub rsp, 1024");
    AddLine(asmbl, "    mov ebx, 0x0");
    AddLine(asmbl, "    mov r8, rdx");
    AddLine(asmbl, "    mov ecx, esi");
    AddLine(asmbl, "    mov edx, esi");
    AddLine(asmbl, "    mov rsi, rsp");
    AddLine(asmbl, "    add rsi, 576");
    AddLine(asmbl, "    mov eax, ecx");
    AddLine(asmbl, "    and eax, 0xf0");
    AddLine(asmbl, "    jz start_sse");

    GenerateCode<kAVX>(asmbl, parsedCode);
    AddLine<kSSE>(asmbl, "; SSE variant");
    GenerateCode<kSSE>(asmbl, parsedCode);

    AddLine(asmbl, "exit:");
    AddLine(asmbl, "    add rsp, 1024");
    AddLine(asmbl, "    mov rbp, [rsp]");
    AddLine(asmbl, "    mov rsp, rbp");
    for(auto iter = regsToSave.rbegin(); iter != regsToSave.rend(); ++iter)
        AddLine(asmbl, "    pop r%u", *iter);
    AddLine(asmbl, "    pop rbx");
    AddLine(asmbl, "    pop rbp");
    AddLine(asmbl, "    ret");
    EmitStoreLoad(asmbl, usedVgprRegistersMask);

    printf("%s", asmbl.c_str());

    if(!Compile(asmbl, parsedCode.groupDimX, parsedCode.groupDimY, outputFile))
        return -1;

    return 0;
}
