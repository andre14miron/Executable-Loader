# Executable Loader

## Description
This project focuses on implementing an executable loader that handles segmentation faults. The goal is to efficiently handle page faults and provide appropriate handling mechanisms for different scenarios.

## Segmentation Fault Handling
The core function implemented in this project is the segv_handler. By utilizing the sigaction structure and examining the si_addr field in the siginfo_t structure, we can effectively handle segmentation faults.

### Case 1: Invalid Memory Access
If the address causing the segmentation fault is not found within any specific segment, it indicates an invalid memory access. In this case, the default page fault handler is executed.

### Case 2: Unauthorized Memory Access
If the page fault occurs within a mapped page but with insufficient permissions, it signifies an unauthorized memory access attempt. Similarly, the default page fault handler is executed in this scenario.

### Case 3: Mapping Unmapped Pages
When the page fault occurs within a segment, and the corresponding page is not yet mapped, it needs to be mapped with the appropriate permissions. Additionally, the entire page is zeroed out, and the necessary data is copied to the newly mapped page.

## Implementation Details
Upon identifying the relevant segment, the loader calculates the page index where the segmentation fault address resides, along with the offset within that page. By determining whether the page has already been mapped, the loader distinguishes between case 2 and case 3 to perform the necessary actions.

To ensure compatibility with the project's requirements, the loader sets the appropriate permissions for the mapped page, allowing for seamless execution of the loaded executables.
