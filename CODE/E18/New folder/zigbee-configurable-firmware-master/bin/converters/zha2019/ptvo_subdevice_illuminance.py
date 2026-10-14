try:
    from homeassistant.components.zha.sensor import (MULTI_MATCH, Illuminance)
    from homeassistant.components.zha.core.const import (CLUSTER_HANDLER_ILLUMINANCE)
except ImportError:
    from zha.application.platforms.sensor import (MULTI_MATCH, Illuminance)
    from zha.zigbee.cluster_handlers.const import (CLUSTER_HANDLER_ILLUMINANCE)

@MULTI_MATCH(
    cluster_handler_names=CLUSTER_HANDLER_ILLUMINANCE,
    models={PTVO_MODEL_ID},
)
class PtvoIlluminanceSensor(Illuminance):
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)
