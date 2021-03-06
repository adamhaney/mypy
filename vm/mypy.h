#ifndef _MYPY_H
#define _MYPY_H

#include <stdlib.h>

typedef unsigned long MValue;
typedef long MSignedValue;
typedef int MBool;

/* TODO do not assume 64 */
#define M_VALUE_BITS 64

typedef struct {
    MValue *frame;
    MValue *stack_top;
} MEnv;

typedef MValue (*MFunction)(MEnv *e);

typedef struct {
    MFunction *vtable;
    int num_slots;
    const char *full_name;
    /* TODO add more information */
} MTypeRepr;

typedef struct {
    MTypeRepr *type;
    MValue gcinfo; /* TODO this is just a placeholder */
} MInstanceHeader;

#define MNone  0x1L
#define MError 0x3L

/* Dummy error handler; used instead of raising an exception for now */
MValue MAbort(MEnv *e);

static inline MValue MAlloc(MEnv *e, size_t size)
{
    // TODO use garbage collector heap
    return (MValue)malloc(size) | 1;
}

static inline MInstanceHeader *MHeader(MValue instance)
{
    return (MInstanceHeader *)(instance - 1L);
}

static inline void MInitInstance(MValue instance, MTypeRepr *type)
{
    MInstanceHeader *h = MHeader(instance);
    h->type = type;
    h->gcinfo = 0;
}

static inline MValue *MSlotPtr(MValue object, int index)
{
    return (MValue *)(MHeader(object) + 1) + index;
}

/* Assume object != MNone */
static inline MValue MGetSlot(MValue object, int index) {
    return *MSlotPtr(object, index);
}

/* Assume object != MNone */
static inline void MSetSlot(MValue object, int index, MValue value) {
    /* TODO use write barrier */
    *MSlotPtr(object, index)  = value;
}

static inline MValue MInvokeVirtual(MEnv *e, MValue receiver,
                                    int vtable_index)
{
    if (receiver == MNone)
        return MAbort(e);
    MInstanceHeader *h = MHeader(receiver);
    return h->type->vtable[vtable_index](e);
}

/* TODO do not assume 64-bit values */
#define M_SHORT_MIN (-0x8000000000000001L - 1)

/* Short ints have the lowest bit unset. */
#define MIsShort(v) (((v) & 1) == 0)

MBool MIntEq(MValue left, MValue right);
MBool MIntNe(MValue left, MValue right);
MBool MIntLe(MValue left, MValue right);
MBool MIntLt(MValue left, MValue right);
MBool MIntGe(MValue left, MValue right);
MBool MIntGt(MValue left, MValue right);
MValue MIntAdd(MEnv *e, MValue x, MValue y);
MValue MIntSub(MEnv *e, MValue x, MValue y);
MValue MIntMul(MEnv *e, MValue x, MValue y);
MValue MIntFloorDiv(MEnv *e, MValue x, MValue y);
MValue MIntMod(MEnv *e, MValue x, MValue y);
MValue MIntAnd(MEnv *e, MValue x, MValue y);
MValue MIntOr(MEnv *e, MValue x, MValue y);
MValue MIntXor(MEnv *e, MValue x, MValue y);
MValue MIntShl(MEnv *e, MValue x, MValue y);
MValue MIntShr(MEnv *e, MValue x, MValue y);

MValue MIntUnaryMinus(MEnv *e, MValue x);
MValue MIntInvert(MEnv *e, MValue x);

/* TODO this is just a trivial dummy print placeholder for test cases */
MValue Mprint(MEnv *e);

static inline MBool MIsAddOverflow(MValue sum, MValue left, MValue right)
{
    return ((MSignedValue)(sum ^ left) < 0 &&
            (MSignedValue)(sum ^ right) < 0);
}

static inline MBool MIsSubOverflow(MValue diff, MValue left, MValue right)
{
    return ((MSignedValue)(diff ^ left) < 0 && 
            (MSignedValue)(diff ^ right) >= 0);
}

/* The multiplication of two non-negative values no larger than this constant
   always fits in a short int. */
#define M_SAFE_MUL (0x80000000L * 2)

static inline MBool MIsPotentialMulOverflow(MValue left, MValue right)
{
    return left > M_SAFE_MUL || right > M_SAFE_MUL;
}

static inline MBool MIsPotentialFloorDivOverflow(MValue left, MValue right)
{
    return (MSignedValue)left < 0 || (MSignedValue)right <= 0;
}

static inline MBool MIsPotentialModOverflow(MValue left, MValue right)
{
    return (MSignedValue)left < 0 || (MSignedValue)right <= 0;
}

static inline MBool MIsShlOverflow(MValue n, MValue s)
{
    return s >= M_VALUE_BITS || ((n << s) >> s) != n;
}

static inline MBool MIsShrOverflow(MValue n, MValue s)
{
    return s >= M_VALUE_BITS || (MSignedValue)s < 0;
}

static inline MBool MShortEq(MValue left, MValue right)
{
    if (left == right)
        return 1;
    else if (MIsShort(left))
        return 0;
    else
        return MIntEq(left, right);
}

static inline MBool MShortNe(MValue left, MValue right)
{
    if (left == right)
        return 0;
    else if (MIsShort(left))
        return 1;
    else
        return MIntNe(left, right);
}

static inline MBool MShortLt(MValue left, MValue right)
{
    if (MIsShort(left) && MIsShort(right))
        return (MSignedValue)left < (MSignedValue)right;
    else
        return MIntLt(left, right);
}

static inline MBool MShortLe(MValue left, MValue right)
{
    if (MIsShort(left) && MIsShort(right))
        return (MSignedValue)left <= (MSignedValue)right;
    else
        return MIntLe(left, right);
}

static inline MBool MShortGt(MValue left, MValue right)
{
    if (MIsShort(left) && MIsShort(right))
        return (MSignedValue)left > (MSignedValue)right;
    else
        return MIntGt(left, right);
}

static inline MBool MShortGe(MValue left, MValue right)
{
    if (MIsShort(left) && MIsShort(right))
        return (MSignedValue)left >= (MSignedValue)right;
    else
        return MIntGe(left, right);
}

MValue Mobject___init__(MEnv *e);

#endif
