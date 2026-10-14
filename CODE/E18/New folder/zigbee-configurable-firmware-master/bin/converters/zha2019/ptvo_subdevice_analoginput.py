ATTR_ID_MEASURED_VALUE = 0x0000

CLUSTER_NAME_GENERIC_ANALOG_VALUE = "analog_input"
CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE = "ptvo_analog_input"
CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE2 = CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE + CLUSTER_NAME_ENDPOINT_SUFFIX

UNIT_MAPPING_CLUSTER = 0
UNIT_MAPPING_ATTR = 1

"""
More info: https://www.home-assistant.io/docs/frontend/icons/
mdi:temperature-celsius
mdi:water-percent
mdi:gauge
mdi:speedometer
mdi:percent
mdi:air-filter
mdi:fan
mdi:flash
mdi:current-ac
mdi:flash
mdi:counter
mdi:thermometer-lines
mdi:timer
mdi:palette
mdi:brightness-percent
"""
try:
    from homeassistant.components.zha.sensor import (MULTI_MATCH, Sensor)
except ImportError:
    from zha.application.platforms.sensor import (MULTI_MATCH, Sensor)

from homeassistant.components.sensor import (
    SensorDeviceClass,
    SensorStateClass,
)

UNIT_CLUSTER_MAPPING = {
    'C': ["temperature", ATTR_ID_MEASURED_VALUE],
    'Pa': ["pressure", ATTR_ID_MEASURED_VALUE],
    'lx': ["illuminance", ATTR_ID_MEASURED_VALUE],
    '%': ["humidity", ATTR_ID_MEASURED_VALUE],
    'V': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "voltage"],
    'A': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "current"],
    'W': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "power"],
    'Hz': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "frequency"],
    'pf': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "power_factor"],
    'Wh': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "energy"],
    'ppm': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "quality"],
    'psize': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "psize"],
    'mcpm0': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "mcpm05"],
    'mcpm1': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "mcpm10"],
    'mcpm2': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "mcpm25"],
    'mcpm4': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "mcpm40"],
    'mcpmA': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "mcpm100"],
    'ncpm0': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "ncpm05"],
    'ncpm1': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "ncpm10"],
    'ncpm2': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "ncpm25"],
    'ncpm4': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "ncpm40"],
    'ncpmA': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "ncpm100"],
    '0': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val0"],
    '1': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val1"],
    '2': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val2"],
    '3': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val3"],
    '4': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val4"],
    '5': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val5"],
    '6': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val6"],
    '7': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val7"],
    '8': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val8"],
    '9': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val9"],
    '10': [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "val10"],
}

class PtvoAnalogInput(CustomCluster, AnalogInput):

    def __init__(self, *args, **kwargs):
        """Init."""
        self._current_state = {}
        self._current_value = 0
        self._current_unit = None
        self._current_dev_addr = None

        super().__init__(*args, **kwargs)

    async def bind(self):
        """Prevent bind."""
        return (zigpy.zcl.foundation.Status.SUCCESS,)

    async def unbind(self):
        """Prevent unbind."""
        return (zigpy.zcl.foundation.Status.SUCCESS,)

    async def _configure_reporting(self, *args, **kwargs):  # pylint: disable=W0221
        """Prevent remote configure reporting."""
        return (zigpy.zcl.foundation.ConfigureReportingResponse.deserialize(b"\x00")[0],)

    def _process_attr_value(self):
        unit = self._current_unit

        if (unit is None) or (unit not in UNIT_CLUSTER_MAPPING):
            handler = [CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE, "value"]
        else:
            handler = UNIT_CLUSTER_MAPPING[unit]
        if handler is None:
            return

        attr_found = False
        destination_cluster_name = ""

        destination_cluster_name = handler[UNIT_MAPPING_CLUSTER] + CLUSTER_NAME_ENDPOINT_SUFFIX + str(self.endpoint.endpoint_id)
        destination_cluster = getattr(self.endpoint, destination_cluster_name, None)
        if destination_cluster is None:
            return

        event_value = self._current_value
        if unit == "C":
            event_value = event_value * 100
        elif unit == "%":
            event_value = event_value * 100
        elif unit == "Pa":
            event_value = event_value * 10

        dest_attr_id = handler[UNIT_MAPPING_ATTR]
        if isinstance(dest_attr_id, str):
            dest_attr_def = destination_cluster.attributes_by_name.get(dest_attr_id, None)
            if dest_attr_def is None:
                _LOGGER.warning("PtvoAnalogInput dest attr not found: ep=%s,name=%s, value=%f", self.endpoint.endpoint_id, dest_attr_id, event_value)
                return
            dest_attr_id = dest_attr_def.id
        if dest_attr_id in destination_cluster.attributes:
            attr_found = True
            destination_cluster._update_attribute(dest_attr_id, event_value)

        if attr_found is False:
            _LOGGER.warning("PtvoAnalogInput cluster or attribute not found: ep=%s,cluster=%s,attr=%s", self.endpoint.endpoint_id, handler[UNIT_MAPPING_CLUSTER], handler[UNIT_MAPPING_ATTR])


    def handle_cluster_general_request(self, hdr, *args, **kwargs):
        """Send write_attributes value to TintRemoteSceneCluster."""
        super().handle_cluster_general_request(hdr, *args, **kwargs)

        if ((hdr.command_id != zigpy.zcl.foundation.GeneralCommand.Report_Attributes) and
         (hdr.command_id != zigpy.zcl.foundation.GeneralCommand.Read_Attributes_rsp)):
            return

        self._process_attr_value()


    def _update_attribute(self, attrid, value):
        super()._update_attribute(attrid, value)

        if value is None:
            return

        if attrid == 85:
            self._current_value = value
            self._current_unit = None
            self._current_dev_addr = None
            return

        if attrid == 28:
            data = value.split(',')
            if len(data) > 0:
                self._current_unit = data[0]
            if len(data) > 1:
                self._current_dev_addr = data[1]

class PtvoAnalogInputReplaceCluster(LocalDataCluster):
    """Generic Analog Value Custom Cluster For All Endpoints."""

    name: str = "Generic Analog Value"
    ep_attribute: str = CLUSTER_NAME_PTVO_ANALOG_INPUT_VALUE2

    def __init__(self, *args, **kwargs):
        """Init."""
        super().__init__(*args, **kwargs)
        self.name = self.name + " Endpoint " + str(self.endpoint.endpoint_id)
        self.ep_attribute = self.ep_attribute + str(self.endpoint.endpoint_id)
        self.cluster_id = 0xFC00 + int(self.endpoint.endpoint_id)

    attributes = {}
    server_commands = {}
    client_commands = {}
