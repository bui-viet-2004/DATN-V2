"""Device handler for _model_ / _description_"""
from zigpy.profiles import zha
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
)

_LOGGER = logging.getLogger(__name__)

PTVO_DEVICE = _manufacturer_id_

_includes_
_converters_

class ptvo_custom_device(CustomDevice):
    """_model_ based on the PTVO firmware."""

    def __init__(self, *args, **kwargs):
        """Init device."""
        super().__init__(*args, **kwargs)

    signature = {
        MODELS_INFO: [("_manufacturer_", "_model_")],
        ENDPOINTS: {
_signature_endpoints_
        },
    }

    replacement = {
        ENDPOINTS: {
_replacement_endpoints_
        },
    }
