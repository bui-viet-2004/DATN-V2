try:
    from homeassistant.components.zha.sensor import (
        MULTI_MATCH, 
        CarbonDioxideConcentration,
        CarbonMonoxideConcentration,
        PM25
    )
except ImportError:
    from zha.application.platforms.sensor import (
        MULTI_MATCH, 
        CarbonDioxideConcentration,
        CarbonMonoxideConcentration,
        PM25
    )

@MULTI_MATCH(
    cluster_handler_names="carbon_dioxide_concentration",
    models={PTVO_MODEL_ID},
)
class PtvoCo2Sensor(CarbonDioxideConcentration):
    """CO2."""
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)


@MULTI_MATCH(
    cluster_handler_names="carbon_monoxide_concentration",
    models={PTVO_MODEL_ID},
)
class PtvoCoSensor(CarbonMonoxideConcentration):
    """CO."""
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)

@MULTI_MATCH(
    cluster_handler_names="pm25",
    models={PTVO_MODEL_ID},
)
class PtvoCoSensor(PM25):
    """PM2.5."""
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)
