.. _nrf70_bm_samples:

Samples
#######

The nRF70 BM repo provides samples that specifically target Nordic Semiconductor devices and show how to implement typical use cases with Nordic Semiconductor libraries and drivers.
Samples showcase a single feature or library.
These samples are a good starting point for understanding how to put together your own application.

The following samples are available:

Scan
++++

The Scan sample demonstrates how to use the Nordic Semiconductor's Wi-Fi® chipset to scan for the access points without using the wpa_supplicant.
For more information, see https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/samples/wifi/scan/README.html

Radio test
++++++++++

The Radio test sample demonstrates how to use the Nordic Semiconductor's Wi-Fi® chipset to test the radio functionality.
For more information, see https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/samples/wifi/radio_test/README.html

Scan and Radio test combo
+++++++++++++++++++++++++

The Scan and Radio test combo sample demonstrates how to switch between the Scan and Radio test operational modes, using different regulatory domains, at runtime.
It does the following:

 * Uses the regulatory domain ``WIFI_REG_DOMAIN``
 * Initializes radio test mode and does continuous TX for 5 seconds, followed by a display of the statistics.
 * Switches to scan mode and scans for the access points.
 * Changes the regulatory domain to ``WIFI_REG_DOMAIN_2`` and repeats the above steps.