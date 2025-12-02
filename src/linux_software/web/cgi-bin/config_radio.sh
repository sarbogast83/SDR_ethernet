#!/bin/bash
# close existing fifoUDPdum if it exists
# --- Set content type for CGI output ---
echo "Content-type: text/plain"
echo ""

echo "Starting configuration..."

# Read all data input cells
read FORM_DATA

ADC_FREQ_HZ=""
TUNE_FREQ_HZ=""
STREAMING="off" # Default to off
TEMP_PAIRS=$(echo "$FORM_DATA" | tr '&' '\n')

# Loop over the temporary variable with default IFS (space/tab/newline)
for PAIR in $TEMP_PAIRS; do
    KEY=$(echo "$PAIR" | cut -d'=' -f1)
    VALUE=$(echo "$PAIR" | cut -d'=' -f2)

    case "$KEY" in
        "adc_freq_hz")
            ADC_FREQ_HZ="$VALUE"
            echo "ADC Freq: $ADC_FREQ_HZ" ;;
        "tune_freq_hz")
            TUNE_FREQ_HZ="$VALUE"
            echo "Tune Freq: $TUNE_FREQ_HZ" ;;
        "streaming")
            STREAMING="on"
            echo "Streaming Status: $STREAMING" ;;
    esac
done


# Extract just the values from each pair using parameter expansion
#ADC_FREQ_HZ=${adc_freq_hz_pair#*=}
#TUNE_FREQ_HZ=${tune_freq_hz_pair#*=}
#STREAMING_STATUS=${streaming_pair#*=}


echo "Writing to memory addresses..."
# direct call to devmem failed, possibly due to varible type
./write_devmem "$ADC_FREQ_HZ" "$TUNE_FREQ_HZ" "$STREAMING"

echo "Configuration complete."

