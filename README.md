# Cache Mechanism Implementation
 [2023-2 Computer Architecture Term Project]

 ## Overview

The program simulates a cache mechanism, processing data access requests and
evaluating the cache’s performance based on hit ratio and bandwidth for different cache
association sizes. The ‘main’ function in ‘main.c’ initializes memory and cache using functions
from ‘cache.c’, reads data access requests from 'access_input.txt,' and employs the
‘retrieve_data’ function to fetch data from cache or memory. The decision is made by the
‘check_cache_data_hit’ function in ‘cache.c’: if there’s a cache hit, data is retrieved from cache
entry based on the specific data type; otherwise, a cache miss returns -1 and prompts the
‘retrieve_data’ function to call ‘access_memory’ for data retrieval from main memory. The
‘access_memory’ function does this by using the entry index from the ‘find_entry_index_in_set’
function. The main function concludes by computing and displaying hit ratio and bandwidth,
and saving accessed data and cache details to “access_output.txt”.
