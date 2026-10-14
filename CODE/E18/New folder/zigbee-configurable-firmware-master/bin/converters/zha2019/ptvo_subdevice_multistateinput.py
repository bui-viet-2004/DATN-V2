from zhaquirks.const import (
    COMMAND,
    COMMAND_HOLD,
    COMMAND_RELEASE,
    COMMAND_SINGLE,
    COMMAND_DOUBLE,
    COMMAND_TRIPLE,
    SHORT_PRESS,
    DOUBLE_PRESS,
    TRIPLE_PRESS,
    LONG_PRESS,
    LONG_RELEASE,
    VALUE,
    ZHA_SEND_EVENT,
)

ACTION_TYPE = {
    0: COMMAND_RELEASE,
    1: COMMAND_SINGLE,
    2: COMMAND_DOUBLE,
    3: COMMAND_TRIPLE,
    4: COMMAND_HOLD
}

class PtvoMultistateInput(CustomCluster, MultistateInput):
    """Multistate input cluster."""

    def __init__(self, *args, **kwargs):
        """Init."""
        self._current_state = {}
        super().__init__(*args, **kwargs)

    async def bind(self):
        """Prevent bind."""
        return (zigpy.zcl.foundation.Status.SUCCESS,)

    async def unbind(self):
        """Prevent unbind."""
        return (zigpy.zcl.foundation.Status.SUCCESS,)

    async def _configure_reporting(self, *args, **kwargs):  # pylint: disable=W0221
        """Prevent remote configure reporting."""
        return (zigpy.zcl.foundation.ConfigureReportingResponse.deserialize(b"\x00")[0],)

    def _update_attribute(self, attrid, value):
        super()._update_attribute(attrid, value)
        if attrid == 0x0055:
            self._current_state[0x0055] = action = ACTION_TYPE.get(value)
            if action is not None:
                event_args = {VALUE: value}
                self.listener_event(ZHA_SEND_EVENT, action, event_args)

            # show something in the sensor in HA
            super()._update_attribute(0, action)
