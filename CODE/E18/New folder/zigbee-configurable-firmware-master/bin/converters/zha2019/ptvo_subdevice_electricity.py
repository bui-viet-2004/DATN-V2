CLUSTER_NAME_PTVO_ELECTRICAL_MEASUREMENT_VALUE = "ptvo_electrical_measurement"
CLUSTER_NAME_PTVO_ELECTRICAL_MEASUREMENT_VALUE2 = CLUSTER_NAME_PTVO_ELECTRICAL_MEASUREMENT_VALUE + CLUSTER_NAME_ENDPOINT_SUFFIX

try:
    from homeassistant.components.zha.sensor import (MULTI_MATCH, Sensor)
    from homeassistant.components.zha import sensor
    from homeassistant.components.zha.core.const import (
        CLUSTER_HANDLER_ELECTRICAL_MEASUREMENT,
        CLUSTER_HANDLER_SMARTENERGY_METERING,
    )
except ImportError:
    from zha.application.platforms.sensor import (MULTI_MATCH, Sensor)
    from zha.application.platforms import sensor
    from zha.zigbee.cluster_handlers.const import (
        CLUSTER_HANDLER_ELECTRICAL_MEASUREMENT,
        CLUSTER_HANDLER_SMARTENERGY_METERING,
    )

from homeassistant.components.sensor import (
    SensorDeviceClass,
    SensorStateClass,
)
import zigpy.types as t

from zigpy.zcl.clusters.homeautomation import ElectricalMeasurement

class PtvoElectricalMeasurementCluster(CustomCluster, ElectricalMeasurement):
    """Custom cluster class for DC power, voltage and current measurement."""

    """Prevent creating sensors"""
    ep_attribute = "not_electrical_measurement"

    DC_VOLTAGE = 0x0100
    DC_CURRENT = 0x0103
    DC_POWER = 0x0106
    DC_VOLTAGE_MULTIPLIER = 0x0200
    DC_VOLTAGE_DIVISOR = 0x0201
    DC_CURRENT_MULTIPLIER = 0x0202
    DC_CURRENT_DIVISOR = 0x0203
    DC_POWER_MULTIPLIER = 0x0204
    DC_POWER_DIVISOR = 0x0205

    AC_VOLTAGE_MULTIPLIER = 0x0600
    AC_VOLTAGE_DIVISOR = 0x0601
    AC_CURRENT_MULTIPLIER = 0x0602
    AC_CURRENT_DIVISOR = 0x0603
    AC_POWER_MULTIPLIER = 0x0604
    AC_POWER_DIVISOR = 0x0605

    REPORT_CONFIG = ()

    _CONSTANT_ATTRIBUTES = {
        DC_VOLTAGE_MULTIPLIER: 1,
        DC_VOLTAGE_DIVISOR: 100,
        DC_CURRENT_MULTIPLIER: 1,
        DC_CURRENT_DIVISOR: 1000,
        DC_POWER_MULTIPLIER: 1,
        DC_POWER_DIVISOR: 10,
        AC_VOLTAGE_MULTIPLIER: 1,
        AC_VOLTAGE_DIVISOR: 100,
        AC_CURRENT_MULTIPLIER: 1,
        AC_CURRENT_DIVISOR: 1000,
        AC_POWER_MULTIPLIER: 1,
        AC_POWER_DIVISOR: 10,
    }

    attributes = ElectricalMeasurement.attributes.copy()
    attributes.update(
        {
            DC_VOLTAGE: ("dc_voltage", t.int16s),
            DC_CURRENT: ("dc_current", t.int16s),
            DC_POWER: ("dc_power", t.int16s),
            DC_VOLTAGE_MULTIPLIER: ("dc_voltage_multiplier", t.uint16_t),
            DC_VOLTAGE_DIVISOR: ("dc_voltage_divisor", t.uint16_t),
            DC_CURRENT_MULTIPLIER: ("dc_current_multiplier", t.uint16_t),
            DC_CURRENT_DIVISOR: ("dc_current_divisor", t.uint16_t),
            DC_POWER_MULTIPLIER: ("dc_power_multiplier", t.uint16_t),
            DC_POWER_DIVISOR: ("dc_power_divisor", t.uint16_t),
        }
    )

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
        return

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

        attr_name = self.attributes.get(attrid).name

        multiplier_attr = self.attributes_by_name[f"{attr_name}_multiplier"]
        divisor_attr = self.attributes_by_name[f"{attr_name}_divisor"]

        multiplier = 1
        divisor = 1

        if (multiplier_attr is not None) and (multiplier_attr.id in self._CONSTANT_ATTRIBUTES):
            multiplier = self._CONSTANT_ATTRIBUTES[multiplier_attr.id]

        if (divisor_attr is not None) and (divisor_attr.id in self._CONSTANT_ATTRIBUTES):
            divisor = self._CONSTANT_ATTRIBUTES[divisor_attr.id]

        event_value = float(value * multiplier) / divisor

        destination_cluster_name = CLUSTER_NAME_PTVO_ELECTRICAL_MEASUREMENT_VALUE2 + str(self.endpoint.endpoint_id)
        destination_cluster = getattr(self.endpoint, destination_cluster_name, None)
        if destination_cluster is None:
            _LOGGER.warning("PtvoElectricalMeasurementCluster destination cluster not found: %s", destination_cluster_name)
            return

        destination_cluster._update_attribute(attrid, event_value)


class PtvoElectricalMeasurementReplaceCluster(LocalDataCluster):
    """Generic Analog Value Custom Cluster For All Endpoints."""

    name: str = "Electrical Measurement"
    ep_attribute: str = CLUSTER_NAME_PTVO_ELECTRICAL_MEASUREMENT_VALUE2

    def __init__(self, *args, **kwargs):
        """Init."""
        super().__init__(*args, **kwargs)
        self.name = self.name + " Endpoint " + str(self.endpoint.endpoint_id)
        self.ep_attribute = self.ep_attribute + str(self.endpoint.endpoint_id)
        self.cluster_id = 0xFB00 + int(self.endpoint.endpoint_id)

    attributes = {}
    server_commands = {}
    client_commands = {}
