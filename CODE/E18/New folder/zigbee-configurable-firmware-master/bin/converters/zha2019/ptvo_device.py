"""Device handler for _model_ / _description_"""
import logging
from zigpy.profiles import zha
import zigpy.types as t
import zigpy.zcl.foundation
from zigpy.quirks import CustomCluster, CustomDevice
from zhaquirks import Bus, LocalDataCluster
from zigpy.zcl.clusters.homeautomation import Diagnostic
from zigpy.zcl.clusters.general import Basic
_requirements_

from zhaquirks.const import (
    DEVICE_TYPE,
    ENDPOINTS,
    INPUT_CLUSTERS,
    MODELS_INFO,
    OUTPUT_CLUSTERS,
    PROFILE_ID,
    ENDPOINT_ID,
    SKIP_CONFIGURATION,
)

_LOGGER = logging.getLogger(__name__)

PTVO_DEVICE_TYPE_GENERIC = 0xFFFE # generic Zigbee device
PTVO_DEVICE_TYPE_ON_OFF_OUTPUT = 0x0002
PTVO_DEVICE_TYPE_LEVEL_CONTROLLABLE_OUTPUT = 0x0003
PTVO_DEVICE_TYPE_REMOTE_CONTROL = 0x0006
PTVO_DEVICE_TYPE_DIMMABLE_LIGHT = 0x0101
PTVO_DEVICE_TYPE_COLORED_DIMMABLE_LIGHT = 0x0102

PTVO_MODEL_ID = "_model_"
CLUSTER_NAME_ENDPOINT_SUFFIX = '_ep'

_includes_
_converters_

class ptvo_custom_device(CustomDevice):
    """_model_ based on the PTVO firmware."""

    def __init__(self, *args, **kwargs):
        """Init device."""
        super().__init__(*args, **kwargs)

    signature = {
        MODELS_INFO: [("_manufacturer_", PTVO_MODEL_ID)],
        ENDPOINTS: {
_signature_endpoints_
        },
    }

    replacement = {
        SKIP_CONFIGURATION: True,
        ENDPOINTS: {
_replacement_endpoints_
        },
    }

    device_automation_triggers = {
_device_automation_triggers_
    }