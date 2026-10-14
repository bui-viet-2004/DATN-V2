tz.ptvo_scd40_command = {
    key: ['command'],
    convertSet: async (entity, key, value, meta) => {
        if (!value) {
            return;
        }
        // attr: presentValue, data type: single precision float
        let write_value = 0;
        switch (value) {
        case 'RESET':
            write_value = 63000;
            break;
        case 'REINIT':
            write_value = 63001;
            break;
        case 'SELF-TEST':
            write_value = 63002;
            break;
        default:
            return;
        }
        const payload = {
            0x55: {
                value: write_value,
                type: 0x39
            }
        };
        await entity.write('genAnalogInput', payload);
    },
    convertGet: async (entity, key, meta) => {
        return;
    },
};

tz.ptvo_scd40_set_env_value = {
    key: ['set_pressure', 'set_temp_offset'],
    convertSet: async (entity, key, value, meta) => {
        if (!value) {
            return;
        }
        // attr: presentValue, data type: single precision float
        const write_value = ((key === 'set_pressure') ? 65000 : 64000) + parseFloat(value);
        const payload = {
            0x55: {
                value: write_value,
                type: 0x39
            }
        };
        await entity.write('genAnalogInput', payload);
    },
    convertGet: async (entity, key, meta) => {
        return;
    },
};