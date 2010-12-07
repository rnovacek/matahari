/* host_private.h - Copyright (C) 2010 Red Hat, Inc.
 * Written by Darryl L. Pierce <dpierce@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
					
typedef struct cpuinfo_
{
  int initialized;
  char *model;
  unsigned int cpus;
  unsigned int cores;
  unsigned int wordsize;
} cpuinfo_t;

extern cpuinfo_t cpuinfo;

extern const char *host_os_get_uuid(void);
extern const char *host_os_get_hostname(void);
extern const char *host_os_get_operating_system(void);
extern const char *host_os_get_hypervisor(void);
extern const char *host_os_get_architecture(void);

extern unsigned int host_os_get_memory(void);
extern unsigned int host_os_get_platform(void);

extern void host_os_reboot(void);
extern void host_os_shutdown(void);
extern void host_os_get_load_averages(double *one, double *five, double *fifteen);

extern void host_os_get_cpu_details(void);