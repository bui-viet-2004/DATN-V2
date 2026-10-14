try:
    from homeassistant.components.zha.sensor import (MULTI_MATCH, Temperature)
    from homeassistant.components.zha.core.const import (CLUSTER_HANDLER_TEMPERATURE)
except ImportError:
    from zha.application.platforms.sensor import (MULTI_MATCH, Temperature)
    from zha.zigbee.cluster_handlers.const import (CLUSTER_HANDLER_TEMPERATURE)

@MULTI_MATCH(
    cluster_handler_names=CLUSTER_HANDLER_TEMPERATURE,
    models={PTVO_MODEL_ID},
)
class PtvoTemperatureSensor(Temperature):
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)
