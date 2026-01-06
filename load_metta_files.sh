#!/bin/bash

INPUT_DIR="/home/marco/das-database-adapter/knowledge_base/context_extended/all_metta_files"
TIMES_FILE="/home/marco/das-database-adapter/knowledge_base/context_extended/execution_times_cluster.txt"
DETAILED_LOG="/home/marco/das-database-adapter/knowledge_base/context_extended/detailed_execution_cluster.log"
TOTAL_START=$(date +%s.%N)
BIN_FILE="db_loader_redis_cluster"

> "$TIMES_FILE"
> "$DETAILED_LOG"

format_time() {
    local seconds=$1
    local hours=$(echo "$seconds / 3600" | bc)
    local minutes=$(echo "($seconds % 3600) / 60" | bc)
    local secs=$(echo "$seconds % 60" | bc)
    
    if [ "$hours" -gt 0 ]; then
        printf "%dh %dm %. 2fs" "$hours" "$minutes" "$secs"
    elif [ "$minutes" -gt 0 ]; then
        printf "%dm %.2fs" "$minutes" "$secs"
    else
        printf "%.3fs" "$secs"
    fi
}

{
    echo "========================================================================"
    echo "BATCH EXECUTION - DV_LOADER"
    echo "========================================================================"
    echo "Start: $(date '+%Y-%m-%d %H:%M:%S')"
    echo "Directory: $INPUT_DIR"
    echo "========================================================================"
    echo ""
} | tee "$TIMES_FILE"

declare -a file_names
declare -a file_times
declare -a file_status

count=0
success=0
failed=0

for file in "$INPUT_DIR"/*; do
    if [ -f "$file" ]; then
        count=$((count + 1))
        filename=$(basename "$file")
        
        echo "[File $count/$(($(ls -1 "$INPUT_DIR" | wc -l)))] Processing: $filename"
        echo "----------------------------------------" >> "$DETAILED_LOG"
        echo "File $count:  $filename" >> "$DETAILED_LOG"
        echo "Start: $(date '+%Y-%m-%d %H:%M:%S')" >> "$DETAILED_LOG"
        
        start=$(date +%s.%N)
        ./scripts/run.sh "$BIN_FILE" "$file" >> "$DETAILED_LOG" 2>&1
        exit_code=$?
        end=$(date +%s.%N)
        
        duration=$(echo "$end - $start" | bc)
        
        file_names+=("$filename")
        file_times+=("$duration")
        
        if [ $exit_code -eq 0 ]; then
            file_status+=("OK")
            success=$((success + 1))
            echo "  ✓ Completed in $(format_time "$duration")"
        else
            file_status+=("FAILED-$exit_code")
            failed=$((failed + 1))
            echo "  ✗ Failed in $(format_time "$duration") - Exit code: $exit_code"
        fi
        
        echo "End: $(date '+%Y-%m-%d %H:%M:%S')" >> "$DETAILED_LOG"
        echo "Duration: $duration seconds" >> "$DETAILED_LOG"
        echo "Exit code: $exit_code" >> "$DETAILED_LOG"
        echo "" >> "$DETAILED_LOG"
    fi
done

TOTAL_END=$(date +%s.%N)
TOTAL_DURATION=$(echo "$TOTAL_END - $TOTAL_START" | bc)

{
    echo ""
    echo "========================================================================"
    echo "FILE BREAKDOWN"
    echo "========================================================================"
    printf "%-5s %-50s %-15s %-10s\n" "No" "File" "Time" "Status"
    echo "------------------------------------------------------------------------"
    
    for i in "${!file_names[@]}"; do
        printf "%-5d %-50s %-15s %-10s\n" \
            "$((i+1))" \
            "${file_names[$i]}" \
            "$(format_time "${file_times[$i]}")" \
            "${file_status[$i]}"
    done
    
    echo ""
    echo "========================================================================"
    echo "STATISTICS"
    echo "========================================================================"
    echo "Total files processed:  $count"
    echo "Successful: $success"
    echo "Failed: $failed"
    
    if [ $count -gt 0 ]; then
        success_rate=$(echo "scale=2; $success * 100 / $count" | bc)
        echo "Success rate: ${success_rate}%"
    fi
    
    echo ""
    echo "Total time: $(format_time "$TOTAL_DURATION")"
    
    if [ $count -gt 0 ]; then
        avg=$(echo "$TOTAL_DURATION / $count" | bc -l)
        echo "Average time per file: $(format_time "$avg")"
        
        min_time=${file_times[0]}
        max_time=${file_times[0]}
        min_idx=0
        max_idx=0
        
        for i in "${!file_times[@]}"; do
            if (( $(echo "${file_times[$i]} < $min_time" | bc -l) )); then
                min_time=${file_times[$i]}
                min_idx=$i
            fi
            if (( $(echo "${file_times[$i]} > $max_time" | bc -l) )); then
                max_time=${file_times[$i]}
                max_idx=$i
            fi
        done
        
        echo ""
        echo "Fastest file: ${file_names[$min_idx]} ($(format_time "$min_time"))"
        echo "Slowest file: ${file_names[$max_idx]} ($(format_time "$max_time"))"
    fi
    
    echo ""
    echo "========================================================================"
    echo "End: $(date '+%Y-%m-%d %H:%M:%S')"
    echo "========================================================================"
    echo ""
    echo "Detailed logs saved in: $DETAILED_LOG"
    
} | tee -a "$TIMES_FILE"

echo ""
echo "✓ Processing completed!"
echo "  - Summary:  $TIMES_FILE"
echo "  - Details: $DETAILED_LOG"
