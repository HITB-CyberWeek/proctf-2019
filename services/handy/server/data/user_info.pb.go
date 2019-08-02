// Code generated by protoc-gen-go. DO NOT EDIT.
// source: user_info.proto

package data

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

type UserInfo struct {
	Username             string   `protobuf:"bytes,1,opt,name=username,proto3" json:"username,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *UserInfo) Reset()         { *m = UserInfo{} }
func (m *UserInfo) String() string { return proto.CompactTextString(m) }
func (*UserInfo) ProtoMessage()    {}
func (*UserInfo) Descriptor() ([]byte, []int) {
	return fileDescriptor_user_info_6c88e35228a422d5, []int{0}
}
func (m *UserInfo) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_UserInfo.Unmarshal(m, b)
}
func (m *UserInfo) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_UserInfo.Marshal(b, m, deterministic)
}
func (dst *UserInfo) XXX_Merge(src proto.Message) {
	xxx_messageInfo_UserInfo.Merge(dst, src)
}
func (m *UserInfo) XXX_Size() int {
	return xxx_messageInfo_UserInfo.Size(m)
}
func (m *UserInfo) XXX_DiscardUnknown() {
	xxx_messageInfo_UserInfo.DiscardUnknown(m)
}

var xxx_messageInfo_UserInfo proto.InternalMessageInfo

func (m *UserInfo) GetUsername() string {
	if m != nil {
		return m.Username
	}
	return ""
}

func init() {
	proto.RegisterType((*UserInfo)(nil), "handy.UserInfo")
}

func init() { proto.RegisterFile("user_info.proto", fileDescriptor_user_info_6c88e35228a422d5) }

var fileDescriptor_user_info_6c88e35228a422d5 = []byte{
	// 90 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x09, 0x6e, 0x88, 0x02, 0xff, 0xe2, 0xe2, 0x2f, 0x2d, 0x4e, 0x2d,
	0x8a, 0xcf, 0xcc, 0x4b, 0xcb, 0xd7, 0x2b, 0x28, 0xca, 0x2f, 0xc9, 0x17, 0x62, 0xcd, 0x48, 0xcc,
	0x4b, 0xa9, 0x54, 0x52, 0xe3, 0xe2, 0x08, 0x05, 0xca, 0x78, 0x02, 0x25, 0x84, 0xa4, 0xb8, 0x38,
	0x40, 0xaa, 0xf2, 0x12, 0x73, 0x53, 0x25, 0x18, 0x15, 0x18, 0x35, 0x38, 0x83, 0xe0, 0x7c, 0x27,
	0xb6, 0x28, 0x96, 0x94, 0xc4, 0x92, 0xc4, 0x24, 0x36, 0xb0, 0x6e, 0x63, 0x40, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xd3, 0x41, 0x3c, 0x3c, 0x50, 0x00, 0x00, 0x00,
}
