const zigbeeHerdsmanConverters = require('zigbee-herdsman-converters');
const zigbeeHerdsmanUtils = require('zigbee-herdsman-converters/lib/utils');
_requirements_

const exposes = zigbeeHerdsmanConverters['exposes'] || require("zigbee-herdsman-converters/lib/exposes");
const ea = exposes.access;
const e = exposes.presets;
const modernExposes = (e.hasOwnProperty('illuminance_lux'))? false: true;
const modernExtend = (modernExposes)?  require('zigbee-herdsman-converters/lib/modernExtend'): false;
const modernExtends = [];

const fz = zigbeeHerdsmanConverters.fromZigbeeConverters || zigbeeHerdsmanConverters.fromZigbee;
const tz = zigbeeHerdsmanConverters.toZigbeeConverters || zigbeeHerdsmanConverters.toZigbee;

const ptvo_switch = (zigbeeHerdsmanConverters.findByModel)?zigbeeHerdsmanConverters.findByModel('ptvo.switch'):zigbeeHerdsmanConverters.findByDevice({modelID: 'ptvo.switch'});
fz.ptvo_on_off = {
  cluster: 'genOnOff',
  type: ['attributeReport', 'readResponse'],
  convert: (model, msg, publish, options, meta) => {
      if (msg.data.hasOwnProperty('onOff')) {
          const channel = msg.endpoint.ID;
          const endpointName = `l${channel}`;
          const binaryEndpoint = model.meta && model.meta.binaryEndpoints && model.meta.binaryEndpoints[endpointName];
          const prefix = (binaryEndpoint) ? model.meta.binaryEndpoints[endpointName] : 'state';
          const property = `${prefix}_${endpointName}`;
	  if (binaryEndpoint) {
            return {[property]: msg.data['onOff'] === 1};
          }
          return {[property]: msg.data['onOff'] === 1 ? 'ON' : 'OFF'};
      }
  },
};

_converters_

const localFromZigbee = [_fromZigbee_];
// for old versions of Z2M
if (fz.ignore_basic_report) {
  localFromZigbee.unshift(fz.ignore_basic_report);
}

const device = {
    zigbeeModel: ['_model_'],
    model: '_model_',
    vendor: '_manufacturer_',
    description: '_description_',
    fromZigbee: localFromZigbee,
    toZigbee: [_toZigbee_],
    exposes: [_exposes_],
    meta: {
        multiEndpoint: true,
        _meta_
    },
    endpoint: (device) => {
        return {
            _endpoint_
        };
    },
    extend: modernExtends,
    _extra_
};

module.exports = device;
