class PtvoColorTemp(CustomCluster, Color):
    """CCT Lighting custom cluster."""

    # Remove RGB color wheel for CCT Lighting: only expose color temperature
    # LIDL bulbs do not correctly report this attribute (comes back as None in Home Assistant)
    _CONSTANT_ATTRIBUTES = {0x400A: 16}
