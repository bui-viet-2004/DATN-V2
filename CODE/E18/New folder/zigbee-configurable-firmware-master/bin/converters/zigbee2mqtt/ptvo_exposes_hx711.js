tz.ptvo_hx711_scale_factor = {
        key: ['scale_factor'],
        convertSet: async (entity, key, value, meta) => {
            if (!value) {
                return;
            }
            // presentValue =  single precision float
            const payload = {0x55: {value: value, type: 0x39}};
            await entity.write('genAnalogInput', payload);
        },
        convertGet: async (entity, key, meta) => { },
};