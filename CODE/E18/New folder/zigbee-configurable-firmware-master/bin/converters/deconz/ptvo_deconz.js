/* global Attr, Item, R, ZclFrame */

function precisionRound(number, precision) {
    const factor = Math.pow(10, precision);
    return Math.round(number * factor) / factor;
}

function processAnalogValue() {
    let val = Attr.val;
    let data1 = '';
    const payloadSize = ZclFrame.payloadSize;
    if (payloadSize > 6) {
        for (let i = 6; i < payloadSize - 4; i++) {
            if ((ZclFrame.at(i) == 0x1C) && (ZclFrame.at(i + 1) == 0x00) && (ZclFrame.at(i + 2) == 0x42)) {
                let descSize = ZclFrame.at(i + 4)
                const descPos = i + 4;
                for (let j = descPos; j < ZclFrame.payloadSize; j++) {
                    data1 += String.fromCharCode(ZclFrame.at(j));
                    descSize -= 1;
                    if(descSize == 0) {
                        break;
                    }
                }
                break;
            }
        }
    }

    let name = '';
    if (data1) {
        const data2 = data1.split(',');
        const devid = data2[1];
        const unit = data2[0];
        if (devid) {
            R.item('state/device').val = devid;
        }
        if (unit) {
            const valRaw = Attr.val;
            val = precisionRound(valRaw, 1);
            const nameLookup = {
                'C': 'temperature',
                '%': 'humidity',
                'm': 'altitude',
                'Pa': 'pressure',
                'ppm': 'quality',
                'psize': 'particle_size',
                'V': 'voltage',
                'A': 'current',
                'Wh': 'energy',
                'W': 'power',
                'Hz': 'frequency',
                'pf': 'power_factor',
                'lx': 'illuminance_lux',
            };
            let nameAlt = '';
            if (unit === 'A' || unit === 'pf') {
                if (valRaw < 1) {
                    val = precisionRound(valRaw, 3);
                }
            }
            if (unit.startsWith('mcpm') || unit.startsWith('ncpm')) {
                const num = unit.substr(4, 1);
                nameAlt = (num === 'A') ? unit.substr(0, 4) + '10' : unit;
                val = precisionRound(valRaw, 2);
            }
            else {
                nameAlt = nameLookup[unit];
            }
            if (nameAlt === undefined) {
                const valueIndex = parseInt(unit, 10);
                if (!isNaN(valueIndex)) {
                    nameAlt = 'val' + unit;
                }
            }
            if (nameAlt !== undefined) {
                name = nameAlt;
            }
        }
    }

    // scale all analog values by N to get N numbers after decimal point in REST API
    let scale = parseInt(R.item('config/scale').val, 10);
    if (!scale){
        scale = 1000;
    }
    val = val * scale;

    if (name) {
        R.item('state/' + name).val = val;
    }
    return val;
}

const type = R.item('attr/type').val;
if (type === 'ZHAConsumption') {
    Item.val = processAnalogValue();
} else {
    Item.val = Attr.val;
}