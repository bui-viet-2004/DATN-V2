tz.ptvo_senseair_calibration = {
        key: ['calibrate'],
        convertSet: async (entity, key, value, meta) => {
            if (!value) {
                return;
            }
            // presentValue =  single precision float
            const write_value = (value === 'ZERO')?0x00:0x01;
            const payload = {0x55: {value: write_value, type: 0x39}};
            await entity.write('genAnalogInput', payload);
        },
        convertGet: async (entity, key, meta) => {
            await entity.read('genAnalogInput', ['presentValue']);
        },
};
