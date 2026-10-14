from zha.application.platforms.sensor import (Humidity)
from zha.zigbee.cluster_handlers.const import (CLUSTER_HANDLER_HUMIDITY)

@register_entity(RelativeHumidity.cluster_id)
class PtvoHumiditySensor(Humidity):
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)

    _cluster_handler_match = ClusterHandlerMatch(
        cluster_handlers=frozenset({CLUSTER_HANDLER_HUMIDITY}),
        models=frozenset({PTVO_MODEL_ID}),
    )
