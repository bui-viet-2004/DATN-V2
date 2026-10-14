tz.ptvo_schedule1 = {
        key: ['schedule'],
        convertSet: async (entity, key, value, meta) => {
            if (!value) {
                return;
            }
            const payload = {4: {value, type: 0x42}}; // activeText, string
            await entity.write('genBinaryValue', payload);
        },
        convertGet: async (entity, key, meta) => {
            await entity.read('genBinaryValue', ['activeText']);
        },
};

tz.ptvo_schedule2 = {
        key: ['on_time'],
        convertSet: async (entity, key, value, meta) => {
            if (value < 0) {
                return;
            }
            const payload = {0x0043: {value, type: 0x23}}; // minimumOnTime, uint32
            await entity.write('genBinaryValue', payload);
        },
        convertGet: async (entity, key, meta) => {
            await entity.read('genBinaryValue', ['minimumOnTime']);
        },
};

fz.ptvo_schedule = {
        cluster: 'genBinaryValue',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            const channel = msg.endpoint.ID;
            const name = `_l${channel}`;
	    const payload = {}
	    payload['outOfService' + name] = msg.data['outOfService'] > 0;
	    payload['error' + name] = msg.data['reliability'];
	    payload['state' + name] = msg.data['presentValue'] > 0;
	    payload['statusFlags' + name] = msg.data['statusFlags'];
	    payload['schedule' + name] = msg.data['activeText'];
	    payload['on_time' + name] = msg.data['minimumOnTime'];
            return payload;
        },
};

const OneJanuary2000 = new Date('January 01, 2000 00:00:00 UTC+00:00').getTime();

fz.ptvo_time = {
        cluster: 'genTime',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            const channel = msg.endpoint.ID;
            const name = `_l${channel}`;
            let data = msg.data['time'];
	    const tz = new Date();
            const dt = new Date(OneJanuary2000 + (data - (tz.getTimezoneOffset() * -1) * 60) * 1000);
	    const payload = {}
            payload['timeText' + name] = dt.toLocaleDateString() + ' ' + dt.toLocaleTimeString();
            return payload;
        },
};

tz.ptvo_time_sync = {
        key: ['timeSync'],
        convertSet: async (entity, key, value, meta) => {
            if (!entity) { return; }
            const dt = new Date();
            // the device supports the local time only
            const time = Math.round((dt.getTime() - OneJanuary2000) / 1000) + (dt.getTimezoneOffset() * -1) * 60;
            // Time-master + synchronised
            const values = {timeStatus: 3, time: time};
            entity.write('genTime', values);
        },
};