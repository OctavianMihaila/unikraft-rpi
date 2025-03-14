/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Santiago Pagani <santiagopagani@gmail.com>
 *
 * This file contains the setup code for the Unikraft platform on Raspberry Pi.
 * It initializes memory, interrupts, and prints debug information.
 * It also retrieves and prints Device Tree (DTB) information.
 */

#include <uk/plat/common/bootinfo.h>
#include <uk/plat/memory.h>
#include <uk/plat/bootstrap.h>
#include <uk/plat/time.h>
#include <arm/cpu.h>
#include <uk/plat/common/sections.h>
#include <raspi/console.h>
#include <raspi/time.h>
#include <raspi/irq.h>
#include <uk/print.h>
#include <uk/arch/types.h>
#include <stdio.h>
#include <raspi/setup.h>

// DTB address from start.S
extern uint64_t dtb_addr;

// Device Tree Header Structure
struct dtb_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
};

// Convert big-endian to little-endian (needed for DTB fields)
static uint32_t dtb_swap_endian(uint32_t val) {
    return ((val >> 24) & 0xFF) | ((val >> 8) & 0xFF00) |
           ((val << 8) & 0xFF0000) | ((val << 24) & 0xFF000000);
}

// Retrieve the DTB address
uint32_t get_dtb_addr(void) {
    return (uint32_t)dtb_addr;
}

// Print DTB Header Information
void print_dtb_header(void *dtb) {
    struct dtb_header *hdr = (struct dtb_header *)dtb;

    // Convert fields from big-endian to little-endian
    uint32_t magic = dtb_swap_endian(hdr->magic);
    uint32_t totalsize = dtb_swap_endian(hdr->totalsize);
    uint32_t off_dt_struct = dtb_swap_endian(hdr->off_dt_struct);
    uint32_t off_dt_strings = dtb_swap_endian(hdr->off_dt_strings);

    uk_pr_info("\n--- Device Tree Header ---\n");
    uk_pr_info("Magic: 0x%x\n", magic);
    uk_pr_info("Total Size: 0x%x\n", totalsize);
    uk_pr_info("DT Structure Offset: 0x%x\n", off_dt_struct);
    uk_pr_info("DT Strings Offset: 0x%x\n", off_dt_strings);
}

// Print raw DTB memory (hex dump for debugging)
void print_raw_dtb(void *dtb, size_t size) {
    unsigned char *data = (unsigned char *)dtb;
    uk_pr_info("\n--- Raw DTB Memory Dump ---\n");

    for (size_t i = 0; i < size; i++) {
        uk_pr_info("%02X ", data[i]);
        if ((i + 1) % 16 == 0)
            uk_pr_info("\n");
    }
    uk_pr_info("\n");
}

// Print readable strings from the DTB strings section
void print_dtb_strings(void *dtb, uint32_t offset) {
    uk_pr_info("\n--- DTB Strings Section ---\n");

    char *str = (char *)(dtb + offset);
    while (*str) {
        uk_pr_info("%s\n", str);
        str += uk_strlen(str) + 1;  // Move to next string
    }
}

static uint64_t assembly_entry;
static uint64_t hardware_init_done;

// Function to get the reset time
uint64_t _libraspiplat_get_reset_time(void) {
    return assembly_entry;
}

// Function to get the hardware init time
uint64_t _libraspiplat_get_hardware_init_time(void) {
    return hardware_init_done;
}

// Memory Initialization
static void __libraspiplat_mem_init(void) {
    struct ukplat_bootinfo *bi = ukplat_bootinfo_get();
    if (unlikely(!bi)) {
        UK_CRASH("Failed to get bootinfo\n");
    }

    // Define memory regions (stack, text, rodata, data, bss, heap)
    int rc;

    rc = ukplat_memregion_list_insert(
        &bi->mrds,
        &(struct ukplat_memregion_desc){
            .vbase = 0,
            .pbase = 0,
            .len   = __TEXT,
            .type  = UKPLAT_MEMRT_RESERVED,
            .flags = UKPLAT_MEMRF_READ | UKPLAT_MEMRF_WRITE,
        });
    if (unlikely(rc < 0))
        uk_pr_err("Failed to add stack memory region descriptor.\n");

    rc = ukplat_memregion_list_insert(
        &bi->mrds,
        &(struct ukplat_memregion_desc){
            .vbase = __TEXT,
            .pbase = __TEXT,
            .len   = (size_t) __ETEXT - (size_t) __TEXT,
            .type  = UKPLAT_MEMRT_RESERVED,
            .flags = UKPLAT_MEMRF_READ,
        });
    if (unlikely(rc < 0))
        uk_pr_err("Failed to add text memory region descriptor.\n");

    rc = ukplat_memregion_list_insert(
        &bi->mrds,
        &(struct ukplat_memregion_desc){
            .vbase = __RODATA,
            .pbase = __RODATA,
            .len   = (size_t) __ERODATA - (size_t) __RODATA,
            .type  = UKPLAT_MEMRT_RESERVED,
            .flags = UKPLAT_MEMRF_READ,
        });
    if (unlikely(rc < 0))
        uk_pr_err("Failed to add rodata memory region descriptor.\n");

    // Add heap
    rc = ukplat_memregion_list_insert(
        &bi->mrds,
        &(struct ukplat_memregion_desc){
            .vbase = __END,
            .pbase = __END,
            .len   = (size_t) ((MMIO_BASE / 2 - 1) - (size_t) __END) / __PAGE_SIZE * __PAGE_SIZE,
            .type  = UKPLAT_MEMRT_FREE,
            .flags = UKPLAT_MEMRF_READ | UKPLAT_MEMRF_WRITE | UKPLAT_MEMRF_MAP,
        });
    if (unlikely(rc < 0))
        uk_pr_err("Failed to add heap memory region descriptor.\n");
}

// Platform Entry Function (called from start.S)
void _libraspiplat_entry(uint64_t low0, uint64_t hi0, uint64_t low1, uint64_t hi1) {
    __libraspiplat_mem_init();
    ukplat_irq_init();

    if (hi0 == hi1) {
        assembly_entry = ((hi0 << 32) & 0xFFFFFFFF00000000) | (low0 & 0xFFFFFFFF);
    } else {
        assembly_entry = ((hi1 << 32) & 0xFFFFFFFF00000000) | (low1 & 0xFFFFFFFF);
    }

    _libraspiplat_init_console();
    hardware_init_done = get_system_timer();

    // Print DTB Address & Info
    uint32_t dtb_address = get_dtb_addr();
    uk_pr_info("DTB Address: 0x%x\n", dtb_address);
    print_dtb_header((void *)dtb_address);
    print_raw_dtb((void *)dtb_address, 512);
    struct dtb_header *hdr = (struct dtb_header *)dtb_address;
    uint32_t dt_strings_offset = dtb_swap_endian(hdr->off_dt_strings);
    print_dtb_strings((void *)dtb_address, dt_strings_offset);

    // Enter Unikraft
    ukplat_entry(0, 0);
}
