    @register_entity(_zcl_base_cluster_id_)
    class _cluster_prefix_SensorEp_endpoint_Attr_attr_dec_(Sensor):
        _attr_name = "_attr_description_ _endpoint_"
        _attribute_name = "_attr_value_id_"
        _attr_translation_key = None
        _unique_id_suffix = "_attr_value_id__ep_endpoint_"
        _attr_icon = "_attr_icon_"
        _attr_device_class = _attr_device_class_
        _attr_state_class = _attr_state_class_
        _attr_native_unit_of_measurement = "_attr_unit_"
        # for newer ZHA version
        icon = "_attr_icon_"
        fallback_name = "_attr_description_ _endpoint_"

        _cluster_handler_match = ClusterHandlerMatch(
            cluster_handlers=frozenset({_cluster_handler_name_ + "_endpoint_"}),
            models=frozenset({PTVO_MODEL_ID}),
        )
