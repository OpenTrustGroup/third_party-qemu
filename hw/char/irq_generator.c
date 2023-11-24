
#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "chardev/char-fe.h"
#include "qemu/log.h"
#include "trace.h"
#include "migration/vmstate.h"

#define TYPE_NAME "irq_generator"
#define IRQ_GENERATOR(obj) OBJECT_CHECK(IrqGeneratorState, (obj), TYPE_NAME)

typedef struct IrqGeneratorState
{
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    qemu_irq irq;
    uint32_t state;
} IrqGeneratorState;

static const VMStateDescription vmstate_irq_generator = {
    .name = "irq_generator",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(state, IrqGeneratorState),
        VMSTATE_END_OF_LIST()
    }
};

static void irq_generator_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->vmsd = &vmstate_irq_generator;
}

static uint64_t irq_generator_read(void *opaque, hwaddr offset,
                                   unsigned size)
{
    IrqGeneratorState *s = (IrqGeneratorState *)opaque;

    switch (offset)
    {
    case 0:
        return s->state;
    default:
        g_assert_not_reached();
        break;
    }

    return 0;
}

static void irq_generator_write(void *opaque, hwaddr offset,
                                uint64_t value, unsigned size)
{
    IrqGeneratorState *s = (IrqGeneratorState *)opaque;

    switch (offset)
    {
    case 0:
        s->state &= ~((uint32_t)value);
        break;
    case 4:
        s->state = (uint32_t)value;
        break;
    default:
        g_assert_not_reached();
        break;
    }

    qemu_set_irq(s->irq, s->state != 0);
}

static const MemoryRegionOps irq_generator_ops = {
    .read = irq_generator_read,
    .write = irq_generator_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void irq_generator_init(Object *obj)
{
    IrqGeneratorState *s = IRQ_GENERATOR(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->iomem, obj, &irq_generator_ops, s, "irq-gen", 0x1000);
    sysbus_init_mmio(dev, &s->iomem);
    sysbus_init_irq(dev, &s->irq);
}

static const TypeInfo irq_generator_info = {
    .name = TYPE_NAME,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(IrqGeneratorState),
    .instance_init = irq_generator_init,
    .class_init = irq_generator_class_init,
};

static void irq_generator_register_types(void)
{
    type_register_static(&irq_generator_info);
}

type_init(irq_generator_register_types)
