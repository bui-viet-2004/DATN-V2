// Copied from fromZigbee.js
// The standard converter does not append the endpoint number

fz.ptvo_humidity = {
  cluster: 'msRelativeHumidity',
  type: ['attributeReport', 'readResponse'],
  options: [exposes.options.precision('humidity'), exposes.options.calibration('humidity')],
  convert: (model, msg, publish, options, meta) => {
      const humidity = parseFloat(msg.data['measuredValue']) / 100.0;

      // https://github.com/Koenkk/zigbee2mqtt/issues/798
      // Sometimes the sensor publishes non-realistic vales, it should only publish message
      // in the 0 - 100 range, don't produce messages beyond these values.
      if (humidity >= 0 && humidity <= 100) {
          const property = zigbeeHerdsmanUtils.postfixWithEndpointName('humidity', msg, model, meta);
          return {[property]: zigbeeHerdsmanUtils.calibrateAndPrecisionRoundOptions(humidity, options, 'humidity')};
      }
  },
};

fz.ptvo_pressure = {
  cluster: 'msPressureMeasurement',
  type: ['attributeReport', 'readResponse'],
  options: [exposes.options.precision('pressure'), exposes.options.calibration('pressure')],
  convert: (model, msg, publish, options, meta) => {
      let pressure = 0;
      if (msg.data.hasOwnProperty('scaledValue')) {
          const scale = msg.endpoint.getClusterAttributeValue('msPressureMeasurement', 'scale');
          pressure = msg.data['scaledValue'] / Math.pow(10, scale) / 100.0; // convert to hPa
      } else {
          pressure = parseFloat(msg.data['measuredValue']);
      }
      const property = zigbeeHerdsmanUtils.postfixWithEndpointName('pressure', msg, model, meta);
      return {[property]: zigbeeHerdsmanUtils.calibrateAndPrecisionRoundOptions(pressure, options, 'pressure')};
  },
};