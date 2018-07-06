#define UNICODE
#define DWIN32_LEAN_AND_MEAN

#pragma comment(lib, "pdh.lib")
#pragma warning(disable: 4668 4820 4710)
// 4668: undefined preprocessor macros in winioctl.h and pdh.h
// 4820: invalid byte paddings in winioctl.h and pdh.h
// 4710: wprintf function not inlined

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <pdh.h>
#include <pdhmsg.h>

void print_phd_error(PDH_STATUS status) {
    // SEE: https://docs.microsoft.com/en-us/windows/desktop/PerfCtrs/pdh-error-codes

    DWORD flags = FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY;
    HMODULE lib = LoadLibrary(L"pdh.dll");
    LPWSTR message = NULL;

    if (lib == NULL)
    {
        wprintf(L"Error %lu: Loading library failed.\n", GetLastError());
        return;
    }

    if (!FormatMessage(
        flags,
        lib,
        (DWORD) status,
        0,
        (LPWSTR) &message,
        0,
        NULL))
    {
        wprintf(L"Error 0x%lx: Formatting error message failed.\n", GetLastError());
        return;
    }

    wprintf(L"Error 0x%lx: %s", status, message);
}

void main(void)
{
    // SEE: https://docs.microsoft.com/en-us/windows/desktop/PerfCtrs/enumerating-process-objects

    LPCWSTR source = NULL; // real-time
    LPCWSTR machine = NULL; // local computer
    PWSTR counter = L"Process";
    PDH_STATUS status = ERROR_SUCCESS;
    LPWSTR counter_list_buffer = NULL;
    DWORD counter_list_size = 0;
    LPWSTR instance_list_buffer = NULL;
    DWORD instance_list_size = 0;
    DWORD details = PERF_DETAIL_WIZARD;
    LPWSTR temp = NULL;

    status = PdhEnumObjectItems(
        source,
        machine,
        counter,
        counter_list_buffer,
        &counter_list_size,
        instance_list_buffer,
        &instance_list_size,
        details,
        0);

    if (status == PDH_MORE_DATA)
    {
        counter_list_buffer = (LPWSTR) malloc(counter_list_size * sizeof(WCHAR));
        instance_list_buffer = (LPWSTR) malloc(instance_list_size * sizeof(WCHAR));

        if (counter_list_buffer == NULL || instance_list_buffer == NULL)
        {
            status = ERROR_OUTOFMEMORY;
        }
        else
        {
            status = PdhEnumObjectItems(
                source,
                machine,
                counter,
                counter_list_buffer,
                &counter_list_size,
                instance_list_buffer,
                &instance_list_size,
                details,
                0);

            if (status == ERROR_SUCCESS)
            {
                wprintf(L"counters:\n\n");
                for (temp = counter_list_buffer; *temp != 0; temp += wcslen(temp) + 1)
                {
                    wprintf(L"%s\n", temp);
                }

                wprintf(L"instances:\n\n");
                for (temp = instance_list_buffer; *temp != 0; temp += wcslen(temp) + 1)
                {
                    wprintf(L"%s\n", temp);
                }
            }
        }
    }

    if (status != ERROR_SUCCESS)
        print_phd_error(status);

    if (counter_list_buffer != NULL)
        free(counter_list_buffer);

    if (instance_list_buffer != NULL)
        free(instance_list_buffer);
}
