/* SPDX-License-Identifier: ISC */
/* Copyright (c) 2015, IBM
 *           (c) 2020, NEC Laboratories Europe GmbH, NEC Corporation.
 * Author(s): Dan Williams <djwillia@us.ibm.com>
 *            Simon Kuenzer <simon.kuenzer@neclab.eu>
 *            Santiago Pagani <santiagopagani@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

int _ukplat_mem_mappings_init(void)
{
	// TODO: The implementation of the __libraspiplat_mem_init function from setup.c should probably be moved here because some common code is calling this to init memory. But it seems like other platfroms are not doing this either?
	return 0;
}
