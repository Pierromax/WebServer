#!/bin/bash

CONFIG_DIR="./config/tests"
WEB_SERVER="./webserv"
LOGS_DIR="./logs"
LOG_FILE="$LOGS_DIR/results.log"

make tester 2>/dev/null
# Check if the web server binary exists
if [ ! -f "$WEB_SERVER" ]; then
    echo "Error: Web server binary not found at $WEB_SERVER."
    exit 1
fi

# Create logs directory if it doesn't exist
mkdir -p "$LOGS_DIR"

# Colors and symbols
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
WHITE="\033[1;37m"
R="\033[0m"
BOLD="\033[1m"
CHECK_MARK="\u2714"
CROSS_MARK="\u274C"

# Initialize counters
total_tests=0
passed_tests=0

# Function to display a message with a blue frame
framed_message() {
    local message="$1"
    local width=$((${#message} + 4))
    local top_border="${BLUE}${BOLD}┌"
    local bottom_border="${BLUE}${BOLD}└"
    
    for ((i=0; i<width-2; i++)); do
        top_border+="─"
        bottom_border+="─"
    done
    
    top_border+="┐${R}"
    bottom_border+="┘${R}"
    
    echo -e "$top_border" | tee -a "$LOG_FILE"
    echo -e "${BLUE}${BOLD}│ $message │${R}" | tee -a "$LOG_FILE"
    echo -e "$bottom_border" | tee -a "$LOG_FILE"
}

# Clear or create the log file
echo "Testing configuration files..." | tee "$LOG_FILE"

# Function to test a single configuration file
test_config() {
    local config_file="$1"
    local invert="$2"
    
    # Increment total test counter
    ((total_tests++))
    
    echo "Testing $config_file..." | tee -a "$LOG_FILE"
    
    # Get the current line number in the log file
    local start_line=$(wc -l < "$LOG_FILE")
    start_line=$((start_line + 1))
    
    # Run the test
    $WEB_SERVER "$config_file" >> "$LOG_FILE" 2>&1
    local result=$?
    
    # Get the ending line number
    local end_line=$(wc -l < "$LOG_FILE")
    local log_ref="${WHITE}${BOLD}→ ${LOG_FILE}:${start_line}-${end_line}${R}"
    
    if [ $result -eq 0 ]; then
        if [ "$invert" == "true" ]; then
            echo -e "${RED}${CROSS_MARK} FAILURE: $config_file passed (unexpected). ${log_ref}" | tee -a "$LOG_FILE"
        else
            echo -e "${GREEN}${CHECK_MARK} SUCCESS: $config_file passed. ${log_ref}" | tee -a "$LOG_FILE"
            # Increment passed test counter
            ((passed_tests++))
        fi
    else
        if [ "$invert" == "true" ]; then
            echo -e "${GREEN}${CHECK_MARK} SUCCESS: $config_file failed (expected). ${log_ref}" | tee -a "$LOG_FILE"
            # Increment passed test counter
            ((passed_tests++))
        else
            echo -e "${RED}${CROSS_MARK} FAILURE: $config_file failed. ${log_ref}" | tee -a "$LOG_FILE"
            
            # Extract error message from the log
            local error_output=$(tail -n $((end_line - start_line + 1)) "$LOG_FILE" | grep -i "error")
            if [ -n "$error_output" ]; then
                echo -e "${YELLOW}  Error: ${error_output}${R}" | tee -a "$LOG_FILE"
            fi
        fi
    fi
    echo "---------------------------------" | tee -a "$LOG_FILE"
}

# Test valid configurations
framed_message "Testing valid configurations..."
find "$CONFIG_DIR/valid" -type f -name "*.conf" | while read -r config; do
    test_config "$config" "false"
done

# Need to save the counters as find+while runs in a subshell
valid_total=$total_tests
valid_passed=$passed_tests

# Test invalid configurations in all subdirectories
framed_message "Testing invalid configurations..."
find "$CONFIG_DIR/invalid" -type f -name "*.conf" | while read -r config; do
    test_config "$config" "true"
done

# Since the commands above run in subshells, we need to recalculate the totals
total_valid=$(find "$CONFIG_DIR/valid" -type f -name "*.conf" | wc -l)
total_invalid=$(find "$CONFIG_DIR/invalid" -type f -name "*.conf" | wc -l)
total_tests=$((total_valid + total_invalid))

# Count passed tests by analyzing the log file
passed_valid=$(grep -c "SUCCESS: .*/valid/.*\.conf passed" "$LOG_FILE")
passed_invalid=$(grep -c "SUCCESS: .*/invalid/.*\.conf failed (expected)" "$LOG_FILE")
passed_tests=$((passed_valid + passed_invalid))

# Calculate percentage
percentage=$((passed_tests * 100 / total_tests))

# Display score in a blue box
framed_message "Score: $passed_tests/$total_tests tests passed ($percentage%)"

echo "Testing completed. Results saved in $LOG_FILE." | tee -a "$LOG_FILE"
