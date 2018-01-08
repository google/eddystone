package com.github.google.beaconfig.utils;

import java.nio.ByteBuffer;
import java.util.Arrays;

import com.github.google.beaconfig.gatt.GattConstants;

/**
 * Class to parse and hold all the broadcast capabilities of a beacon
 */
public class BroadcastCapabilities {
    private int version;
    private int maxSupportedTotalSlots;
    private int maxSupportedEidSlots;
    private byte[] supportedTxPowers;

    private boolean isVariableTxPowerSupported;
    private boolean isVariableAdvSupported;

    private boolean isUidSupported;
    private boolean isUrlSupported;
    private boolean isTlmSupported;
    private boolean isEidSupported;

    public BroadcastCapabilities(byte[] data) {
        ByteBuffer buf = ByteBuffer.wrap(data);
        version = buf.get();
        maxSupportedTotalSlots = (int) buf.get();
        maxSupportedEidSlots = (int) buf.get();

        byte capabilitiesBitField = buf.get();
        isVariableAdvSupported =
                (capabilitiesBitField & GattConstants.CAPABILITIES_IS_VARIABLE_ADV_SUPPORTED)
                        == GattConstants.CAPABILITIES_IS_VARIABLE_ADV_SUPPORTED;
        isVariableTxPowerSupported =
                (capabilitiesBitField
                        & GattConstants.CAPABILITIES_IS_VARIABLE_TX_POWER_SUPPORTED)
                        == GattConstants.CAPABILITIES_IS_VARIABLE_TX_POWER_SUPPORTED;

        short supportedFrameTypesBitField = buf.getShort();
        isUidSupported =
                (supportedFrameTypesBitField & GattConstants.CAPABILITIES_IS_UID_FRAME_SUPPORTED)
                        == GattConstants.CAPABILITIES_IS_UID_FRAME_SUPPORTED;
        isUrlSupported =
                (supportedFrameTypesBitField & GattConstants.CAPABILITIES_IS_URL_FRAME_SUPPORTED)
                        == GattConstants.CAPABILITIES_IS_URL_FRAME_SUPPORTED;
        isTlmSupported =
                (supportedFrameTypesBitField & GattConstants.CAPABILITIES_IS_TLM_FRAME_SUPPORTED)
                        == GattConstants.CAPABILITIES_IS_TLM_FRAME_SUPPORTED;
        isEidSupported =
                (supportedFrameTypesBitField & GattConstants.CAPABILITIES_IS_EID_FRAME_SUPPORTED)
                        == GattConstants.CAPABILITIES_IS_EID_FRAME_SUPPORTED;

        supportedTxPowers = Arrays.copyOfRange(data, buf.position(), data.length);
        Arrays.sort(supportedTxPowers);
    }

    public int getVersion() {
        return version;
    }

    public int getMaxSupportedTotalSlots() {
        return maxSupportedTotalSlots;
    }

    public int getMaxSupportedEidSlots() {
        return maxSupportedEidSlots;
    }

    public byte[] getSupportedTxPowers() {
        return supportedTxPowers;
    }

    public boolean isVariableTxPowerSupported() {
        return isVariableTxPowerSupported;
    }

    public boolean isVariableAdvSupported() {
        return isVariableAdvSupported;
    }

    public boolean isUidSupported() {
        return isUidSupported;
    }

    public boolean isUrlSupported() {
        return isUrlSupported;
    }

    public boolean isTlmSupported() {
        return isTlmSupported;
    }

    public boolean isEidSupported() {
        return isEidSupported;
    }
}
