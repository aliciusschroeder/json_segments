#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cJSON.h>

#include "json_segments.h"

// Initialize the global function pointer for JSON processing to NULL.
// This ensures it's explicitly set by the user before use.
JsonProcessingFunction current_json_processing_function = NULL;

// Initialize the global pointer for storing JSON segment information to NULL.
// This will be allocated memory as segments are added.
JsonSegmentInfo *all_json_segments = NULL;

int all_json_segments_count = 0;

// Add a JSON segment to the global array. This function searches for the
// unique_id in all_json_segments. If found, it adds the segment to the
// existing JsonSegmentInfo structure. If not found, it creates a new entry.
void json_segments_add(const char *unique_id, int sequence_number, int total_segments, const char *json_segment) {
    // Search for unique_id in all_json_segments
    for (int i = 0; i < all_json_segments_count; i++) {
        // Check if we already received segments of the same unique id
        if (strcmp(all_json_segments[i].unique_id, unique_id) == 0) {
            // Check data consistency
            if (all_json_segments[i].total_segments != total_segments) {
                fprintf(stderr, "Error: Inconsistent total number of segments\n");
                return;
            }

            // Check if the sequence_number already exists
            for (int j = 0; j < all_json_segments[i].received_segments; j++) {
                if (all_json_segments[i].segments[j].sequence_number == sequence_number) {
                    // Segment already received, return without adding
                    return;
                }
            }

            // Add segment to existing
            int index = all_json_segments[i].received_segments;
            all_json_segments[i].segments[index].sequence_number = sequence_number;
            all_json_segments[i].segments[index].json_segment = strdup(json_segment);
            all_json_segments[i].received_segments++;
            all_json_segments[i].last_received_timestamp = time(NULL);

            // Check if JSON segments for this uid are complete now
            if (all_json_segments[i].received_segments == all_json_segments[i].total_segments) {
                json_segments_merge(unique_id);
            }
            return;
        }
    }

    // Create new entry, if unique_id does not exist yet
    JsonSegmentInfo *temp = realloc(all_json_segments, sizeof(JsonSegmentInfo) * (all_json_segments_count + 1));
    if (temp == NULL) {
        fprintf(stderr, "Memory allocation error!\n");
        return NULL;
    } else {
        all_json_segments = temp;
    }

    int idx = all_json_segments_count;
    all_json_segments[idx].unique_id = strdup(unique_id);
    all_json_segments[idx].total_segments = total_segments;
    all_json_segments[idx].received_segments = 1;
    all_json_segments[idx].segments = malloc(sizeof(JsonSegment) * total_segments);
    all_json_segments[idx].segments[0].sequence_number = sequence_number;
    all_json_segments[idx].segments[0].json_segment = strdup(json_segment);
    all_json_segments[idx].last_received_timestamp = time(NULL);
    all_json_segments_count++;

    return;
}


// Parse a cJSON object and add its contents as a segment. The function
// extracts the unique_id, sequence number, total segments, and the segment
// string from the cJSON object, validating each field before adding the segment.
void json_segments_parse_input(cJSON *json_obj) {
    if (json_obj == NULL) {
        fprintf(stderr, "Ungültiges cJSON-Objekt\n");
        return;
    }

    cJSON *uid = cJSON_GetObjectItem(json_obj, "uid");
    cJSON *seq = cJSON_GetObjectItem(json_obj, "seq");
    cJSON *abs = cJSON_GetObjectItem(json_obj, "abs");
    cJSON *seg = cJSON_GetObjectItem(json_obj, "seg");

    if (!cJSON_IsString(uid) || !cJSON_IsNumber(seq) || !cJSON_IsNumber(abs) || !cJSON_IsString(seg)) {
        fprintf(stderr, "JSON-Objekt enthält ungültige Daten\n");
        return;
    }

    json_segments_add(uid->valuestring, seq->valueint, abs->valueint, seg->valuestring);
}

// Create a single JSON segment object. This function constructs a cJSON object
// with the given UID, sequence number, total segments, and content, structuring
// it as per the expected JSON segment format.
void json_segments_create_single(cJSON **root, char *uid, int sequence_number, int total_segments, char *content) {
    *root = cJSON_CreateObject();
    cJSON_AddStringToObject(*root, "uid", uid);
    cJSON_AddNumberToObject(*root, "seq", sequence_number);
    cJSON_AddNumberToObject(*root, "abs", total_segments);
    cJSON_AddStringToObject(*root, "seg", content);
}

// Calculate the overhead of a JSON segment. This helper function creates a
// temporary cJSON object with dummy values to estimate the additional space
// required for metadata in each JSON segment.
int json_segments_overhead_size(const char *uid, int seq, int abs) {
    // Create a temporary cJSON object to calculate the overhead
    cJSON *temp = cJSON_CreateObject();
    cJSON_AddStringToObject(temp, "uid", uid);
    cJSON_AddNumberToObject(temp, "seq", seq);
    cJSON_AddNumberToObject(temp, "abs", abs);
    cJSON_AddStringToObject(temp, "seg", "");

    char *temp_str = cJSON_PrintUnformatted(temp);
    int overhead = strlen(temp_str) - 2; // minus 2 for the empty seg string

    free(temp_str);
    cJSON_Delete(temp);

    return overhead;
}

// Split a string into multiple JSON segments. This function divides a given
// string into segments of a specified maximum length, considering the
// overhead of JSON formatting, and creates cJSON objects for each segment.
cJSON **json_segments_split_string(const char *str, const char *uid, int max_length) {
    if (str == NULL || uid == NULL || max_length <= 0) {
        return NULL;
    }

    int total_length = strlen(str);
    int overhead = json_segments_overhead_size(uid, 0, 0); // Calculate overhead with dummy values
    int max_seg_length = max_length - overhead;

    if (max_seg_length <= 0) {
        return NULL; // The max_length is too small even for the overhead
    }

    int total_segments = (total_length + max_seg_length - 1) / max_seg_length;
    cJSON **segments = malloc(sizeof(cJSON *) * total_segments);

    if (segments == NULL) {
        return NULL; // Memory allocation failure
    }

    for (int i = 0; i < total_segments; i++) {
        int start_idx = i * max_seg_length;
        int end_idx = start_idx + max_seg_length;
        if (end_idx > total_length) {
            end_idx = total_length;
        }

        char *seg_str = strndup(str + start_idx, end_idx - start_idx);

        json_segments_create_single(&segments[i], (char *)uid, i + 1, total_segments, seg_str);

        free(seg_str);
    }

    return segments;
}

// Free the memory allocated for an array of cJSON segments. This function
// ensures that all cJSON objects in the array are safely deleted and the
// memory for the array itself is freed. It relies on the first segment's
// 'abs' field to determine the total number of segments.
void json_segments_free_segments_array(cJSON **segments) {
    if (segments == NULL) {
        return; // Nothing to free
    }

    // Assuming the first segment is not NULL and it has the 'abs' field correctly set
    cJSON *first_segment = segments[0];
    cJSON *abs_item = cJSON_GetObjectItem(first_segment, "abs");
    if (!cJSON_IsNumber(abs_item)) {
        // Error handling if 'abs' is not a number or does not exist
        fprintf(stderr, "Error: 'abs' field is missing or not a number in the first segment\n");
        return;
    }

    int num_segments = abs_item->valueint;

    // Iterate through the cJSON objects and delete them
    for (int i = 0; i < num_segments; i++) {
        cJSON_Delete(segments[i]);
    }

    // Free the array of pointers itself
    free(segments);
}

// Delete all segments associated with a given unique_id. This function
// frees all resources associated with the segments, including memory
// for the unique ID, segment strings, and the segment array itself.
void json_segments_delete_segments(const char *unique_id) {
    for (int i = 0; i < all_json_segments_count; i++) {
        if (strcmp(all_json_segments[i].unique_id, unique_id) == 0) {
            // Free all ressources
            free(all_json_segments[i].unique_id);
            for (int j = 0; j < all_json_segments[i].received_segments; j++) {
                free(all_json_segments[i].segments[j].json_segment);
            }
            free(all_json_segments[i].segments);

            // Move the remaining elements to avoid gaps
            for (int j = i; j < all_json_segments_count - 1; j++) {
                all_json_segments[j] = all_json_segments[j + 1];
            }

            all_json_segments_count--;
            
            JsonSegmentInfo *temp = realloc(all_json_segments, sizeof(JsonSegmentInfo) * all_json_segments_count);
            if (temp == NULL) {
                fprintf(stderr, "Memory allocation error!\n");
                return NULL;
            } else {
                all_json_segments = temp;
            }

            return;
        }
    }
}

// Check for and handle timeouts in receiving JSON segments. This function
// iterates through all JSON segments and deletes those that have not been
// completed within the specified timeout period.
void json_segments_check_timeout(int timeout) {
    time_t current_time = time(NULL);
    for (int i = 0; i < all_json_segments_count; i++) {
        double seconds_diff = difftime(current_time, all_json_segments[i].last_received_timestamp);
        if (seconds_diff > timeout) {
            json_segments_delete_segments(all_json_segments[i].unique_id);
            // Nach dem Löschen eines Elements, iteriere erneut vom aktuellen Index
            i--;
        }
    }
}

// Interpret a complete JSON object after reassembly. This function calls
// the user-defined JSON processing function set in the global function pointer.
// If no function is set, it logs an error.
void json_segments_process_merged(cJSON *json) {
    if (current_json_processing_function != NULL) {
        current_json_processing_function(json);
    } else {
        fprintf(stderr, "Keine Verarbeitungsfunktion gesetzt\n");
    }
}

// Merge all received segments associated with a unique_id into a complete JSON object.
// This function sorts the segments in order, concatenates them into a single string,
// and parses it into a cJSON object. The complete JSON is then passed to json_segments_process_merged.
void json_segments_merge(const char *unique_id) {
    for (int i = 0; i < all_json_segments_count; i++) {
        if (strcmp(all_json_segments[i].unique_id, unique_id) == 0) {
            // Check if all segments have been received
            if (all_json_segments[i].received_segments != all_json_segments[i].total_segments) {
                return;
            }

            // Sort the segments with Insertion Sort
            for (int j = 1; j < all_json_segments[i].total_segments; j++) {
                JsonSegment key = all_json_segments[i].segments[j];
                int k = j - 1;

                // Move elements of all_json_segments[i].segments[0..j-1] that are greater than key
                while (k >= 0 && all_json_segments[i].segments[k].sequence_number > key.sequence_number) {
                    all_json_segments[i].segments[k + 1] = all_json_segments[i].segments[k];
                    k = k - 1;
                }
                all_json_segments[i].segments[k + 1] = key;
            }

            char *full_json_str = NULL;
            int total_length = 0;

            // Determine the total length of the combined string
            for (int j = 0; j < all_json_segments[i].total_segments; j++) {
                total_length += strlen(all_json_segments[i].segments[j].json_segment);
            }

            // Allocate memory for the complete string
            full_json_str = (char *)malloc(total_length + 1);
            if (full_json_str == NULL) {
                fprintf(stderr, "Memory allocation error\n");
                return;
            }

            // Merge segments
            strcpy(full_json_str, all_json_segments[i].segments[0].json_segment);
            for (int j = 1; j < all_json_segments[i].total_segments; j++) {
                strcat(full_json_str, all_json_segments[i].segments[j].json_segment);
            }

            // Parse the merged JSON
            cJSON *json = cJSON_Parse(full_json_str);
            if (json == NULL) {
                fprintf(stderr, "Fehler beim Parsen von JSON\n");
                free(full_json_str);
                return;
            }

            json_segments_process_merged(json);

            cJSON_Delete(json);
            free(full_json_str);

            // Remove processed segments
            json_segments_delete_segments(unique_id);
            return;
        }
    }
}