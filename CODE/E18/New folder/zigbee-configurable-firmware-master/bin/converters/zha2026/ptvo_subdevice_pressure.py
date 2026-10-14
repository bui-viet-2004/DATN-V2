from zha.application.platforms.sensor import (Pressure)
from zha.zigbee.cluster_handlers.const import (CLUSTER_HANDLER_PRESSURE)

@register_entity(PressureMeasurement.cluster_id)
class PtvoPressureSensor(Pressure):
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)

    _cluster_handler_match = ClusterHandlerMatch(
        cluster_handlers=frozenset({CLUSTER_HANDLER_PRESSURE}),
        models=frozenset({PTVO_MODEL_ID}),
    )
