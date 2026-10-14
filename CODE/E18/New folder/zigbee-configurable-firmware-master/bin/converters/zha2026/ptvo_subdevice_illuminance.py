from zha.application.platforms.sensor import (Illuminance)
from zha.zigbee.cluster_handlers.const import (CLUSTER_HANDLER_ILLUMINANCE)

@register_entity(IlluminanceMeasurement.cluster_id)
class PtvoIlluminanceSensor(Illuminance):
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)

    _cluster_handler_match = ClusterHandlerMatch(
        cluster_handlers=frozenset({CLUSTER_HANDLER_ILLUMINANCE}),
        models=frozenset({PTVO_MODEL_ID}),
    )
