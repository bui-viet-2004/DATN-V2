tz.ptvo_acs712_command = {
    key: ['command'],
    convertSet: async (entity, key, value, meta) => {
        if (!value) {
            return;
        }
        let write_value = 0;
        switch (value) {
        case 'ZERO':
            write_value = 0;
            break;
        default:
            return;
        }
        // presentValue =  single precision float
        const payload = {0x55: {value: write_value, type: 0x39}};
        await entity.write('genAnalogInput', payload);
    },
    convertGet: async (entity, key, meta) => {
        return;
    },
};
