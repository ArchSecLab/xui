#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <spdk/env.h>
#include <spdk/ioat.h>
#include <spdk/log.h>
#include <m5_mmap.h>
#include <gem5/m5ops.h>

#define SRC_SIZE 1024
#define DST_SIZE 1024
#define COPY_SIZE 1024
static void ioat_memcpy_complete(void *arg)
{
    int *done = arg;
    *done = 1;
}

struct spdk_ioat_chan *ioat_chan = NULL;

static bool probe_cb(void *cb_ctx, struct spdk_pci_device *pci_device)
{
    if (ioat_chan)
    {
        return false;
    }
    else
    {
        return true;
    }
}

static void attach_cb(void *cb_ctx, struct spdk_pci_device *pci_device, struct spdk_ioat_chan *ioat)
{
    // Check if that device/channel supports copy operations
    if (!(spdk_ioat_get_dma_capabilities(ioat) & SPDK_IOAT_ENGINE_COPY_SUPPORTED))
    {
        fprintf(stderr, "OH no the device does not support copy operations\n");
        return;
    }

    ioat_chan = ioat;
    printf("Attaching to the ioat device!\n");
}

int main()
{
    // Initialize SPDK
    struct spdk_env_opts opts;

    m5op_addr = 0xFFFF0000;
    map_m5_mem();
    m5_switch_cpu_addr();

    for (int i = 0; i < 2 * 1000 * 6; i++)
    {
        for (int j = 0; j < 12 * 6; j++)
        {
            asm volatile("nop");
        }
    }

    spdk_env_opts_init(&opts);
    opts.name = "ioat_memcpy";
    if (spdk_env_init(&opts) < 0)
    {
        fprintf(stderr, "Unable to initialize SPDK env\n");
        return 1;
    }
    int ret = spdk_ioat_probe(NULL, probe_cb, attach_cb);
    if (ret)
    {
        fprintf(stderr, "Unable to get IOAT channel\n");
        return 1;
    }
    // Allocate memory for src and dst
    uint8_t *src = (uint8_t *)(COPY_SIZE << 1, sizeof(uint64_t), NULL);
    uint8_t *dst = src + COPY_SIZE;

    // Initialize source buffer with some data
    memset(src, 0xAA, COPY_SIZE);
    memset(dst, 0xBB, COPY_SIZE);

    int done;

    // Submit the IOAT memcpy operation
    if (spdk_ioat_submit_copy(ioat_chan, &done, ioat_memcpy_complete, dst, src, COPY_SIZE))
    {
        fprintf(stderr, "IOAT copy submission failed\n");
        return 1;
    }

    // Poll for completion
    while (!done)
    {
        spdk_ioat_process_events(ioat_chan);
    }

    // Verify the copy
    if (memcmp(src, dst, COPY_SIZE) == 0)
    {
        printf("Memcpy completed successfully!\n");
    }
    else
    {
        printf("Memcpy failed!\n");
    }

    return 0;
}