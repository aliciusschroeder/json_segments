// json_segments.h

/*
Getting started:
JsonProcessingFunction current_json_processing_function = NULL;

void process_json(cJSON *json) {
    // Logic for processing the JSON object
}

int main() {
    current_json_processing_function = process_json;
}

*/

/**
 * @file json_segments.h
 * @brief Header file for JSON segmentation and reassembly functions.
 *
 * This file contains declarations and data structures used for segmenting and reassembling JSON objects.
 * It is designed to handle large JSON objects that are split into smaller segments for processing.
 */

#ifndef JSON_SEGMENTS_H
#define JSON_SEGMENTS_H

#include <cJSON.h>
#include <time.h>

// Typedef for a function pointer for JSON processing
typedef void (*JsonProcessingFunction)(cJSON *);

/**
 * @brief Global function pointer for JSON processing.
 * 
 * This function pointer should be set to a user-defined function that processes the reassembled JSON object.
 */
extern JsonProcessingFunction current_json_processing_function;

/**
 * @brief Structure representing a small segment of a JSON object.
 */
typedef struct {
    int sequence_number;    ///< Sequence number of the JSON segment.
    char *json_segment;     ///< String containing the JSON segment.
} JsonSegment;

/**
 * @brief Structure representing information about all segments of a JSON object.
 */
typedef struct {
    char *unique_id;                        ///< Unique identifier for the collection of JSON segments.
    int received_segments;                  ///< Number of segments received so far.
    int total_segments;                     ///< Total number of segments expected.
    JsonSegment *segments;            ///< Array of JSON segments.
    time_t last_received_timestamp;         ///< Timestamp of the last received segment.
} JsonSegmentInfo;

// Global array of all JSON segment information
extern JsonSegmentInfo *all_json_segments;
extern int all_json_segments_count;

/**
 * @brief Add a JSON segment to the global array.
 * 
 * @param unique_id Unique identifier for the JSON object.
 * @param sequence_number Sequence number of the segment.
 * @param total_segments Total number of segments in the JSON object.
 * @param json_segment String containing the JSON segment.
 */
void json_segments_add(const char *unique_id, int sequence_number, int total_segments, const char *json_segment);

/**
 * @brief Delete all segments associated with a unique_id.
 * 
 * @param unique_id Unique identifier for the JSON object.
 */
void json_segments_delete_segments(const char *unique_id);

/**
 * @brief Check for and handle timeouts in receiving JSON segments.
 * 
 * @param timeout Time in seconds to consider a segment as timed out.
 */
void json_segments_check_timeout(int timeout);

/**
 * @brief Merge all received segments associated with a unique_id into a complete JSON object.
 * 
 * @param unique_id Unique identifier for the JSON object.
 */
void json_segments_merge(const char *unique_id);

/**
 * @brief Parse a cJSON object and add its contents as a segment.
 * 
 * @param json_obj cJSON object to parse and add.
 */
void json_segments_parse_input(cJSON *json_obj);

/**
 * @brief Interpret a complete JSON object after reassembly.
 * 
 * @param json cJSON object to interpret.
 *
 * void interpret_json(cJSON *json);
*/

/**
 * @brief Create a single JSON segment object.
 * 
 * @param root Pointer to the cJSON object root.
 * @param uid Unique identifier for the JSON object.
 * @param sequence_number Sequence number of the segment.
 * @param total_segments Total number of segments in the JSON object.
 * @param content String containing the segment content.
 */
void json_segments_create_single(cJSON **root, char *uid, int sequence_number, int total_segments, char *content);

/**
 * @brief Split a string into multiple JSON segments.
 * 
 * @param str String to be split into segments.
 * @param uid Unique identifier for the JSON object.
 * @param max_length Maximum length of each segment.
 * @return Array of cJSON objects representing the segments.
 */
cJSON **json_segments_split_string(const char *str, const char *uid, int max_length);


/**
 * @brief Frees the memory allocated for an array of cJSON segments.
 * 
 * This function is responsible for safely deallocating memory used by an array of cJSON objects
 * representing JSON segments. It should be called to clean up memory once the segments are no longer needed.
 * 
 * @attention It is assumed that the first segment is not NULL and that it correctly represents the total number of segments
 * in its 'abs' field. If this is not the case, the function may not correctly free all allocated memory, leading to memory leaks.
 *
 * @param segments Pointer to the array of cJSON objects representing JSON segments.
 */
void json_segments_free_segments_array(cJSON **segments);


#endif // JSON_SEGMENTS_H
