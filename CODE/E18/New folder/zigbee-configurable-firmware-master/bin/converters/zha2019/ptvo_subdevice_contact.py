from zhaquirks.const import (ZONE_STATUS, ZONE_TYPE)

OCCUPANCY = 0x0000

class PtvoOnOffContact(CustomCluster):
    cluster_id = OnOff.cluster_id
    ep_attribute = "ptvo_on_off"  # prevent creating a visual control

    attributes = OnOff.attributes.copy()

    server_commands = OnOff.server_commands.copy()
    client_commands = OnOff.client_commands.copy()

    def _update_attribute(self, attrid, value):
        if value is None:
            return

        if attrid != 0:
            return

        if value:
            ias_value = 1
        else:
            ias_value = 0

        cluster = getattr(self.endpoint, IasZone.ep_attribute, None)
        if cluster is not None:
            self.endpoint.ias_zone.update_attribute(ZONE_STATUS, ias_value)
            return

        cluster = getattr(self.endpoint, OccupancySensing.ep_attribute, None)
        if cluster is not None:
            self.endpoint.occupancy.update_attribute(OCCUPANCY, ias_value)

    async def bind(self):
        """Prevent bind."""
        return (zigpy.zcl.foundation.Status.SUCCESS,)

    async def unbind(self):
        """Prevent unbind."""
        return (zigpy.zcl.foundation.Status.SUCCESS,)

    async def _configure_reporting(self, *args, **kwargs):  # pylint: disable=W0221
        """Prevent remote configure reporting."""
        return (zigpy.zcl.foundation.ConfigureReportingResponse.deserialize(b"\x00")[0],)


class PtvoIasContact(CustomCluster, IasZone):
    _CONSTANT_ATTRIBUTES = {ZONE_TYPE: IasZone.ZoneType.Contact_Switch}


class PtvoIasGas(CustomCluster, IasZone):
    _CONSTANT_ATTRIBUTES = {ZONE_TYPE: IasZone.ZoneType.Carbon_Monoxide_Sensor}


class PtvoIasNoiseDetected(CustomCluster, IasZone):
    _CONSTANT_ATTRIBUTES = {ZONE_TYPE: IasZone.ZoneType.Standard_Warning_Device}


class PtvoIasOccupancy(CustomCluster, OccupancySensing):
    ''' PtvoIasOccupancy '''

class PtvoIasPresence(CustomCluster, OccupancySensing):
    ''' PtvoIasPresence '''

class PtvoIasSmoke(CustomCluster, IasZone):
    _CONSTANT_ATTRIBUTES = {ZONE_TYPE: IasZone.ZoneType.Fire_Sensor}


class PtvoIasSos(CustomCluster, IasZone):
    _CONSTANT_ATTRIBUTES = {ZONE_TYPE: IasZone.ZoneType.Personal_Emergency_Device}


class PtvoIasTamper(CustomCluster, IasZone):
    _CONSTANT_ATTRIBUTES = {ZONE_TYPE: IasZone.ZoneType.Glass_Break_Sensor}


class PtvoIasVibration(CustomCluster, IasZone):
    _CONSTANT_ATTRIBUTES = {ZONE_TYPE: IasZone.ZoneType.Vibration_Movement_Sensor}


class PtvoIasWaterLeak(CustomCluster, IasZone):
    _CONSTANT_ATTRIBUTES = {ZONE_TYPE: IasZone.ZoneType.Water_Sensor}
