from zha.application.platforms.sensor import (Temperature)
from zha.zigbee.cluster_handlers.const import (CLUSTER_HANDLER_TEMPERATURE)

@register_entity(TemperatureMeasurement.cluster_id)
class PtvoTemperatureSensor(Temperature):
    @property
    def name(self):
        # append the endpoint number to separate similar sensors on different endpoints
        return super().name  + ' ' + str(self._cluster_handler.cluster.endpoint.endpoint_id)

    _cluster_handler_match = ClusterHandlerMatch(
        cluster_handlers=frozenset({CLUSTER_HANDLER_TEMPERATURE}),
        models=frozenset({PTVO_MODEL_ID}),
    )
