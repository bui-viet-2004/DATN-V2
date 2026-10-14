try:
    from homeassistant.components.zha.sensor import (MULTI_MATCH, Pressure)
    from homeassistant.components.zha.core.const import (CLUSTER_HANDLER_PRESSURE)
except ImportError:
    from zha.application.platforms.sensor import (MULTI_MATCH, Pressure)
    from zha.zigbee.cluster_handlers.const import (CLUSTER_HANDLER_PRESSURE)

@MULTI_MATCH(
    cluster_handler_names=CLUSTER_HANDLER_PRESSURE,
    models={PTVO_MODEL_ID},
)
class PtvoPressureSensor(Pressure):
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)
