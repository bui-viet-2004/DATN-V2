from zha.application.platforms.sensor import (
    CarbonDioxideConcentration,
    CarbonMonoxideConcentration,
    PM25
)

@register_entity(CarbonDioxideConcentrationCluster.cluster_id)
class PtvoCo2Sensor(CarbonDioxideConcentration):
    """CO2."""
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)

    _cluster_handler_match = ClusterHandlerMatch(
        cluster_handlers=frozenset({"carbon_dioxide_concentration"}),
        models=frozenset({PTVO_MODEL_ID}),
    )


@register_entity(CarbonMonoxideConcentrationCluster.cluster_id)
class PtvoCoSensor(CarbonMonoxideConcentration):
    """CO."""
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)

    _cluster_handler_match = ClusterHandlerMatch(
        cluster_handlers=frozenset({"carbon_monoxide_concentration"}),
        models=frozenset({PTVO_MODEL_ID}),
    )


@register_entity(PM25Cluster.cluster_id)
class PtvoCoSensor(PM25):
    """PM2.5."""
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)

    _cluster_handler_match = ClusterHandlerMatch(
        cluster_handlers=frozenset({"pm25"}),
        models=frozenset({PTVO_MODEL_ID}),
    )
