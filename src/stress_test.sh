#!/bin/bash

# Default parameters
DATA_TYPE="int"
LIST_SIZE=1000
ITERATIONS=100
BUILD_TYPE="release"

# Parse command line options
while getopts "t:s:i:b:h" opt; do
  case $opt in
    t)
      DATA_TYPE="$OPTARG"
      ;;
    s)
      LIST_SIZE="$OPTARG"
      ;;
    i)
      ITERATIONS="$OPTARG"
      ;;
    b)
      BUILD_TYPE="$OPTARG"
      ;;
    h)
      echo "Usage: $0 [-t data_type] [-s list_size] [-i iterations] [-b build_type]"
      echo "  -t data_type   : Type of data to sort (int or string) [default: int]"
      echo "  -s list_size   : Size of the list to sort [default: 1000]"
      echo "  -i iterations  : Number of test iterations [default: 100]"
      echo "  -b build_type  : Build type (release, debug, test) [default: release]"
      echo ""
      echo "Example: $0 -t int -s 5000 -i 200 -b release"
      exit 0
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

shift $((OPTIND -1))

# Validate data type
if [[ "$DATA_TYPE" != "int" && "$DATA_TYPE" != "string" ]]; then
  echo "Error: Data type must be 'int' or 'string'"
  exit 1
fi

# Validate list size
if [[ "$LIST_SIZE" -le 0 ]]; then
  echo "Error: List size must be greater than 0"
  exit 1
fi

# Validate iterations
if [[ "$ITERATIONS" -le 0 ]]; then
  echo "Error: Iterations must be greater than 0"
  exit 1
fi

# Build the project
echo "Building project with BUILD=$BUILD_TYPE..."
make BUILD=$BUILD_TYPE
if [[ $? -ne 0 ]]; then
  echo "Build failed!"
  exit 1
fi

# Determine executable path
case $BUILD_TYPE in
  release)
    EXECUTABLE="./build/release/myapp"
    ;;
  debug)
    EXECUTABLE="./build/debug/myapp_d"
    ;;
  test)
    EXECUTABLE="./build/tests/myapp_t"
    ;;
  *)
    echo "Unknown build type: $BUILD_TYPE"
    exit 1
    ;;
esac

# Check if executable exists
if [[ ! -f "$EXECUTABLE" ]]; then
  echo "Error: Executable not found at $EXECUTABLE"
  exit 1
fi

# Remove any existing results file
rm -f results.txt

echo "================================================"
echo "Threading Stress Test"
echo "================================================"
echo "Data Type:    $DATA_TYPE"
echo "List Size:    $LIST_SIZE"
echo "Iterations:   $ITERATIONS"
echo "Build Type:   $BUILD_TYPE"
echo "Executable:   $EXECUTABLE"
echo "================================================"
echo ""
echo "Running $ITERATIONS iterations..."
echo "Results will be saved in results.txt"
echo "Please wait..."
echo ""

# Spinner characters
sp="/-\|"
i=0
success_count=0
fail_count=0

# Progress bar function
print_progress() {
  local current=$1
  local total=$2
  local percent=$((current * 100 / total))
  local filled=$((percent / 2))
  local empty=$((50 - filled))
  
  printf "\r["
  printf "%${filled}s" | tr ' ' '='
  printf "%${empty}s" | tr ' ' ' '
  printf "] %3d%% (%d/%d) ${sp:i++%${#sp}:1}" "$percent" "$current" "$total"
}

# Run iterations
for iter in $(seq 1 $ITERATIONS); do
  output=$($EXECUTABLE $DATA_TYPE $LIST_SIZE 2>&1)
  echo "$output" >> results.txt
  
  # Check if the output contains "List is sorted!"
  if echo "$output" | grep -q "List is sorted!"; then
    ((success_count++))
  else
    ((fail_count++))
    echo "" >> results.txt
    echo "=== FAILURE at iteration $iter ===" >> results.txt
    echo "" >> results.txt
  fi
  
  print_progress $iter $ITERATIONS
done

echo ""
echo ""
echo "================================================"
echo "Test Results"
echo "================================================"

line_count=$(wc -l < results.txt)
echo "Total lines in results.txt: $line_count"
echo "Successful sorts:           $success_count"
echo "Failed sorts:               $fail_count"

if [[ $fail_count -eq 0 ]]; then
  echo ""
  echo "✓ All tests passed successfully!"
  echo "================================================"
  exit 0
else
  echo ""
  echo "✗ Some tests failed. Check results.txt for details."
  echo "================================================"
  exit 1
fi