try:
    from homeassistant.components.zha.sensor import (MULTI_MATCH, Humidity)
    from homeassistant.components.zha.core.const import (CLUSTER_HANDLER_HUMIDITY)
except ImportError:
    from zha.application.platforms.sensor import (MULTI_MATCH, Humidity)
    from zha.zigbee.cluster_handlers.const import (CLUSTER_HANDLER_HUMIDITY)

@MULTI_MATCH(
    cluster_handler_names=CLUSTER_HANDLER_HUMIDITY,
    stop_on_match_group=CLUSTER_HANDLER_HUMIDITY,
    models={PTVO_MODEL_ID},
)
class PtvoHumiditySensor(Humidity):
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)
