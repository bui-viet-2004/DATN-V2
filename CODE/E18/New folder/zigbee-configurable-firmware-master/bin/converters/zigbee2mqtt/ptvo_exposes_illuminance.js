// Copied from fromZigbee.js
// The standard converter does not append the endpoint number

function ptvo_illuminance_options () {
  var res = [exposes.options.calibration('illuminance', 'percentual')];
  if (!modernExposes) {
    res.push(exposes.options.calibration('illuminance_lux', 'percentual'));
  }
  return res;
}

fz.ptvo_illuminance = {
    cluster: 'msIlluminanceMeasurement',
    type: ['attributeReport', 'readResponse'],
    options: ptvo_illuminance_options,
    convert: (model, msg, publish, options, meta) => {
        // multi-endpoint version based on the stastard onverter 'fz.illuminance'
        // DEPRECATED: only return lux here (change illuminance_lux -> illuminance)
        const illuminance = msg.data['measuredValue'];
        const illuminanceLux = illuminance === 0 ? 0 : Math.pow(10, (illuminance - 1) / 10000);
        const multiEndpoint = model.meta && model.meta.hasOwnProperty('multiEndpoint') && model.meta.multiEndpoint;
        const property1 = (multiEndpoint)? zigbeeHerdsmanUtils.postfixWithEndpointName('illuminance', msg, model, meta): 'illuminance';
        const property2 = (multiEndpoint)? zigbeeHerdsmanUtils.postfixWithEndpointName('illuminance_lux', msg, model, meta): 'illuminance_lux';
        if (modernExposes) {
            return {
                [property1]: zigbeeHerdsmanUtils.calibrateAndPrecisionRoundOptions(illuminanceLux, options, 'illuminance'),
            };
        }
        return {
            [property1]: zigbeeHerdsmanUtils.calibrateAndPrecisionRoundOptions(illuminance, options, 'illuminance'),
            [property2]: zigbeeHerdsmanUtils.calibrateAndPrecisionRoundOptions(illuminanceLux, options, 'illuminance_lux'),
        };
    },
};