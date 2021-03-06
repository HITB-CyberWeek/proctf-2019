// Code generated by protoc-gen-go. DO NOT EDIT.
// source: task_info.proto

package data

import (
	fmt "fmt"
	proto "github.com/golang/protobuf/proto"
	math "math"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion3 // please upgrade the proto package

type TaskInfo struct {
	RequesterId          string   `protobuf:"bytes,1,opt,name=requester_id,json=requesterId,proto3" json:"requester_id,omitempty"`
	MasterId             string   `protobuf:"bytes,2,opt,name=master_id,json=masterId,proto3" json:"master_id,omitempty"`
	Title                string   `protobuf:"bytes,3,opt,name=title,proto3" json:"title,omitempty"`
	Description          string   `protobuf:"bytes,4,opt,name=description,proto3" json:"description,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *TaskInfo) Reset()         { *m = TaskInfo{} }
func (m *TaskInfo) String() string { return proto.CompactTextString(m) }
func (*TaskInfo) ProtoMessage()    {}
func (*TaskInfo) Descriptor() ([]byte, []int) {
	return fileDescriptor_b9bbeaa14c05050f, []int{0}
}

func (m *TaskInfo) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_TaskInfo.Unmarshal(m, b)
}
func (m *TaskInfo) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_TaskInfo.Marshal(b, m, deterministic)
}
func (m *TaskInfo) XXX_Merge(src proto.Message) {
	xxx_messageInfo_TaskInfo.Merge(m, src)
}
func (m *TaskInfo) XXX_Size() int {
	return xxx_messageInfo_TaskInfo.Size(m)
}
func (m *TaskInfo) XXX_DiscardUnknown() {
	xxx_messageInfo_TaskInfo.DiscardUnknown(m)
}

var xxx_messageInfo_TaskInfo proto.InternalMessageInfo

func (m *TaskInfo) GetRequesterId() string {
	if m != nil {
		return m.RequesterId
	}
	return ""
}

func (m *TaskInfo) GetMasterId() string {
	if m != nil {
		return m.MasterId
	}
	return ""
}

func (m *TaskInfo) GetTitle() string {
	if m != nil {
		return m.Title
	}
	return ""
}

func (m *TaskInfo) GetDescription() string {
	if m != nil {
		return m.Description
	}
	return ""
}

func init() {
	proto.RegisterType((*TaskInfo)(nil), "handy.TaskInfo")
}

func init() { proto.RegisterFile("task_info.proto", fileDescriptor_b9bbeaa14c05050f) }

var fileDescriptor_b9bbeaa14c05050f = []byte{
	// 150 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x09, 0x6e, 0x88, 0x02, 0xff, 0xe2, 0xe2, 0x2f, 0x49, 0x2c, 0xce,
	0x8e, 0xcf, 0xcc, 0x4b, 0xcb, 0xd7, 0x2b, 0x28, 0xca, 0x2f, 0xc9, 0x17, 0x62, 0xcd, 0x48, 0xcc,
	0x4b, 0xa9, 0x54, 0x6a, 0x62, 0xe4, 0xe2, 0x08, 0x01, 0x4a, 0x79, 0x02, 0x65, 0x84, 0x14, 0xb9,
	0x78, 0x8a, 0x52, 0x0b, 0x4b, 0x53, 0x8b, 0x4b, 0x52, 0x8b, 0xe2, 0x33, 0x53, 0x24, 0x18, 0x15,
	0x18, 0x35, 0x38, 0x83, 0xb8, 0xe1, 0x62, 0x9e, 0x29, 0x42, 0xd2, 0x5c, 0x9c, 0xb9, 0x89, 0x30,
	0x79, 0x26, 0xb0, 0x3c, 0x07, 0x44, 0x00, 0x28, 0x29, 0xc2, 0xc5, 0x5a, 0x92, 0x59, 0x92, 0x93,
	0x2a, 0xc1, 0x0c, 0x96, 0x80, 0x70, 0x84, 0x14, 0xb8, 0xb8, 0x53, 0x52, 0x8b, 0x93, 0x8b, 0x32,
	0x0b, 0x4a, 0x32, 0xf3, 0xf3, 0x24, 0x58, 0x20, 0x86, 0x22, 0x09, 0x39, 0xb1, 0x45, 0xb1, 0xa4,
	0x24, 0x96, 0x24, 0x26, 0xb1, 0x81, 0x9d, 0x66, 0x0c, 0x08, 0x00, 0x00, 0xff, 0xff, 0xd2, 0xc6,
	0x77, 0x31, 0xad, 0x00, 0x00, 0x00,
}
