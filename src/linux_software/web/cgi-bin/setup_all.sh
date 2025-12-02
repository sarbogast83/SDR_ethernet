#!/bin/bash
# close existing fifoUDPdum if it exists
PID=$(pgrep -f fifoUDPdump) # > /dev/null
if [ -n "$PID" ]; then
    echo "killing fifo dump <br>"
	kill "$PID" 
    sleep 2 # wait for kill to complete
	echo "killed <br>"
fi

TARGET_IP=$(echo "$QUERY_STRING" | sed -n 's/.*target_ip=\([^&]*\).*/\1/p')

# --- Add a Debug Check Immediately After Extraction ---

if [ -z "$TARGET_IP" ]; then
    echo Content-type: text/html
    echo
    echo "<h1>Error: Failed to extract Target IP address.</h1>"
    echo "<p>The script received an empty value for 'target_ip'. Check your HTML form submission.</p>"
    # Exit the script here since we can't continue
    exit 1
fi

echo "Content-type: text/html" # Tells the browser what kind of content to expect
echo "" # An empty line. Mandatory, if it is missed the page content will not load
echo "<p><em>"
echo "loading PL...<br>"
fpgautil -b design_1_wrapper.bit.bin
# write memory to deactivate radio
devmem 0x43c00008 w 2
devmem 0x43c00008 w 0
echo "</p></em><p>"
echo "configuring Codec...<br>"
./configure_codec.sh
echo "</p>"

# close existing fifoUDPdum if it exists
PID=$(pgrep -f fifoUDPdump) # > /dev/null
if [ -n "$PID" ]; then
    echo "killing fifo dump <br>"
	kill "$PID" 
    sleep 2 # wait for kill to complete
	echo "killed <br>"
fi
# start fifo UDP send packets
echo "Initializing UDP to $TARGET_IP<br>"
nohup ./fifoUDPdump $TARGET_IP > output.log 2>&1 &
echo "<p><em>All Done!</em></p>" 
echo "<p> USE "Radio Tuning" to enable UDP stream!<p>"
