//
// Generated file, do not edit! Created by opp_msgtool 6.0 from BunkerPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "BunkerPacket_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace inet {

Register_Class(BunkerPacket)

BunkerPacket::BunkerPacket() : ::inet::FieldsChunk()
{
}

BunkerPacket::BunkerPacket(const BunkerPacket& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

BunkerPacket::~BunkerPacket()
{
}

BunkerPacket& BunkerPacket::operator=(const BunkerPacket& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void BunkerPacket::copy(const BunkerPacket& other)
{
    this->type = other.type;
    this->survivorName = other.survivorName;
    this->ip = other.ip;
    this->bunkerId = other.bunkerId;
    this->textMessage = other.textMessage;
}

void BunkerPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->type);
    doParsimPacking(b,this->survivorName);
    doParsimPacking(b,this->ip);
    doParsimPacking(b,this->bunkerId);
    doParsimPacking(b,this->textMessage);
}

void BunkerPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->type);
    doParsimUnpacking(b,this->survivorName);
    doParsimUnpacking(b,this->ip);
    doParsimUnpacking(b,this->bunkerId);
    doParsimUnpacking(b,this->textMessage);
}

int BunkerPacket::getType() const
{
    return this->type;
}

void BunkerPacket::setType(int type)
{
    handleChange();
    this->type = type;
}

const char * BunkerPacket::getSurvivorName() const
{
    return this->survivorName.c_str();
}

void BunkerPacket::setSurvivorName(const char * survivorName)
{
    handleChange();
    this->survivorName = survivorName;
}

const L3Address& BunkerPacket::getIp() const
{
    return this->ip;
}

void BunkerPacket::setIp(const L3Address& ip)
{
    handleChange();
    this->ip = ip;
}

int BunkerPacket::getBunkerId() const
{
    return this->bunkerId;
}

void BunkerPacket::setBunkerId(int bunkerId)
{
    handleChange();
    this->bunkerId = bunkerId;
}

const char * BunkerPacket::getTextMessage() const
{
    return this->textMessage.c_str();
}

void BunkerPacket::setTextMessage(const char * textMessage)
{
    handleChange();
    this->textMessage = textMessage;
}

class BunkerPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_type,
        FIELD_survivorName,
        FIELD_ip,
        FIELD_bunkerId,
        FIELD_textMessage,
    };
  public:
    BunkerPacketDescriptor();
    virtual ~BunkerPacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(BunkerPacketDescriptor)

BunkerPacketDescriptor::BunkerPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::BunkerPacket)), "inet::FieldsChunk")
{
    propertyNames = nullptr;
}

BunkerPacketDescriptor::~BunkerPacketDescriptor()
{
    delete[] propertyNames;
}

bool BunkerPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<BunkerPacket *>(obj)!=nullptr;
}

const char **BunkerPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *BunkerPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int BunkerPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 5+base->getFieldCount() : 5;
}

unsigned int BunkerPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_type
        FD_ISEDITABLE,    // FIELD_survivorName
        0,    // FIELD_ip
        FD_ISEDITABLE,    // FIELD_bunkerId
        FD_ISEDITABLE,    // FIELD_textMessage
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *BunkerPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "type",
        "survivorName",
        "ip",
        "bunkerId",
        "textMessage",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int BunkerPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "type") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "survivorName") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "ip") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "bunkerId") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "textMessage") == 0) return baseIndex + 4;
    return base ? base->findField(fieldName) : -1;
}

const char *BunkerPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_type
        "string",    // FIELD_survivorName
        "inet::L3Address",    // FIELD_ip
        "int",    // FIELD_bunkerId
        "string",    // FIELD_textMessage
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **BunkerPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *BunkerPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int BunkerPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void BunkerPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'BunkerPacket'", field);
    }
}

const char *BunkerPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string BunkerPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        case FIELD_type: return long2string(pp->getType());
        case FIELD_survivorName: return oppstring2string(pp->getSurvivorName());
        case FIELD_ip: return pp->getIp().str();
        case FIELD_bunkerId: return long2string(pp->getBunkerId());
        case FIELD_textMessage: return oppstring2string(pp->getTextMessage());
        default: return "";
    }
}

void BunkerPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        case FIELD_type: pp->setType(string2long(value)); break;
        case FIELD_survivorName: pp->setSurvivorName((value)); break;
        case FIELD_bunkerId: pp->setBunkerId(string2long(value)); break;
        case FIELD_textMessage: pp->setTextMessage((value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'BunkerPacket'", field);
    }
}

omnetpp::cValue BunkerPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        case FIELD_type: return pp->getType();
        case FIELD_survivorName: return pp->getSurvivorName();
        case FIELD_ip: return omnetpp::toAnyPtr(&pp->getIp()); break;
        case FIELD_bunkerId: return pp->getBunkerId();
        case FIELD_textMessage: return pp->getTextMessage();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'BunkerPacket' as cValue -- field index out of range?", field);
    }
}

void BunkerPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        case FIELD_type: pp->setType(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_survivorName: pp->setSurvivorName(value.stringValue()); break;
        case FIELD_bunkerId: pp->setBunkerId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_textMessage: pp->setTextMessage(value.stringValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'BunkerPacket'", field);
    }
}

const char *BunkerPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr BunkerPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        case FIELD_ip: return omnetpp::toAnyPtr(&pp->getIp()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void BunkerPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    BunkerPacket *pp = omnetpp::fromAnyPtr<BunkerPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'BunkerPacket'", field);
    }
}

}  // namespace inet

namespace omnetpp {

}  // namespace omnetpp

