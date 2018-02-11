// Copyright (c) 1994-2018 Matt Samudio (Albert Lea Data)  All Rights Reserved.
// Contact information for Albert Lea Data available at http://www.albertleadata.org
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libusb-1.0/libusb.h>

//	Core functions
int main (int, char **);
int setKBPrefs( struct libusb_context *, char *);
void setColor( libusb_device_handle *, unsigned char, unsigned char, unsigned char);
int detectMSiKB( libusb_device_handle *, struct libusb_device_descriptor *);

//	Color utility functions
unsigned char parseRed( char *);
unsigned char parseGreen( char *);
unsigned char parseBlue( char *);
unsigned char parseColor( char *, int);

//	Exploratory prototype functions (not needed)
void genDevSummary( struct libusb_context *);
void showInfo( libusb_device_handle *, struct libusb_device_descriptor *);
void showInfoX( libusb_device *);


int main (int argc, char **argv) {
	struct libusb_context *pCtx=0L;
	char sRGB[7]; strcpy( sRGB, "00FF00");		// Green is default
	unsigned char bHelp=0x01;
//	Process command-line arguments
	for ( int i=1; i < argc; i++) {
		if ( !memcmp( argv[i], "--rgb=", 6)) {
			memcpy( sRGB, &(argv[i][6]), 6); sRGB[6]='\0';
			bHelp=0x00;
		}
	}
//	Prepare to speak USB
	int iRC = libusb_init( &pCtx);
//	printf( "DBG: iRC=%d\n", iRC);
	libusb_set_debug( pCtx, 3);
	if ( setKBPrefs( pCtx, sRGB) && bHelp) {
		printf( "\n");
		printf( "You can specify color with a command-line parameter that\n");
		printf( "looks like this:\n");
		printf( "\n");
		printf( "	--rgb=FFFFFF\n");
		printf( "\n");
		printf( "... which would set the keyboard color to white.\n");
		printf( "The 6-digit hex value represents typical RGB notation.\n");
		printf( "\n");
		printf( "If your keyboard is detected, its color will now be set\n");
		printf( "to green, which is the default (preferred by the author).\n");
		printf( "\n");
	}
//	genDevSummary( pCtx);
	libusb_exit( 0L);
	return( 0);
}

int setKBPrefs( struct libusb_context *pCtx, char *sRGB) {
	int iRet=1;
	libusb_device **pDevs;
	ssize_t iDevs = libusb_get_device_list( pCtx, &pDevs); //get the list of devices
//	printf( "DBG: iDevs=%d\n", iDevs);
	if ( iDevs > 0) {
		libusb_device_handle *pDev = libusb_open_device_with_vid_pid( pCtx, 6000, 65280);
		if ( pDev != 0L) {
			libusb_free_device_list( pDevs, 1);
			if ( libusb_kernel_driver_active( pDev, 0) == 1) {
				if ( libusb_detach_kernel_driver( pDev, 0) == 0) printf( "DBG: driver detached\n");
			}
			if ( libusb_set_configuration( pDev, 1) < 0) printf( "DBG: can't config\n");
			if ( libusb_claim_interface( pDev, 0) < 0) printf( "DBG: can't claim\n");
		//	Make changes to KB state
		//	printf( "DBG: setting colors from RGB=%s\n", sRGB);
			setColor( pDev, parseRed(sRGB), parseGreen(sRGB), parseBlue(sRGB));
		//	Clean up
			libusb_release_interface( pDev, 0);
			libusb_close( pDev);
		} else {
			iRet = 0;
			printf( "\n");
			printf( "You might need to run the program as root (via sudo) ...\n");
			printf( "\n");
		}
	}
	return( iRet);
}

	
void setColor(	libusb_device_handle *pDev,
					unsigned char bR, unsigned char bG, unsigned char bB) {
	int iOut=0;
	uint8_t iReqType = LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT;
	uint8_t iReq = 0x09;				// HID set_report
	uint16_t iVal = (3 << 8)|1;	// HID feature
	unsigned char pData[8];
	pData[0] = 1;
	pData[1] = 2;
	pData[2] = 64;						// Set color
	pData[3] = 2;						// middle
	pData[4] = bR;						// R
	pData[5] = bG;						// G
	pData[6] = bB;						// B
	pData[7] = 0;
//	Left
	pData[3] = 1;
	libusb_control_transfer( pDev, iReqType, iReq, iVal, 0, (unsigned char *)pData, 8, 1000);
//	Middle
	pData[3] = 2;
	libusb_control_transfer( pDev, iReqType, iReq, iVal, 0, (unsigned char *)pData, 8, 1000);
//	Right
	pData[3] = 3;
	libusb_control_transfer( pDev, iReqType, iReq, iVal, 0, (unsigned char *)pData, 8, 1000);
	return;
}

int detectMSiKB( libusb_device_handle *pDev, struct libusb_device_descriptor *pDInf) {
	int iRet=0;
	const int iTxtSz=256;
	unsigned char sTxt[iTxtSz];
	if ( pDInf && (pDInf->iManufacturer > 0)) {
		int iRC = libusb_get_string_descriptor_ascii( pDev,pDInf->iManufacturer,sTxt,iTxtSz);
		if ( (iRC > 0) && !memcmp( sTxt, "MSI EPF USB", 12)) {
//			printf( "DBG:	manufacturer '%s' found ...\n", sTxt);
			if ( pDInf->iProduct > 0) {
				iRC = libusb_get_string_descriptor_ascii(	pDev, pDInf->iProduct, sTxt, iTxtSz);
				if ( (iRC > 0) && !memcmp( sTxt, "MSI EPF USB", 12)) {
//					printf( "DBG:	product '%s' found ...\n", sTxt);
					iRet = 1;
				}
			}
		} // else printf( "DBG: get product failed\n");
	}
	return( iRet);
}

unsigned char parseRed( char *sRGB) { return( parseColor( sRGB, 1)); }
unsigned char parseGreen( char *sRGB) { return( parseColor( sRGB, 2)); }
unsigned char parseBlue( char *sRGB) { return( parseColor( sRGB, 3)); }
unsigned char parseColor( char *sRGB, int iHue) {
	unsigned char bRet=0x00;
	char sHex[3]; sHex[2]='\0';
	int iX=-1;
	switch ( iHue) {
		case 1: iX=0; break;
		case 2: iX=2; break;
		case 3: iX=4; break;
	};
	if ( iX >= 0) {
		unsigned char bVal=0x00;
		sHex[0]=sRGB[iX];
		sHex[1]=sRGB[iX+1];
//		printf( "DBG: sHex=%s\n", sHex);
		if ( sscanf( sHex, "%hhx", &bVal) == 1) {
			bRet=bVal;
//			printf( "DBG: bRet=%hhu\n", bRet);
		}
	}
	return( bRet);
}

void genDevSummary( struct libusb_context *pCtx) {
	libusb_device **pDevs = NULL;
	ssize_t iDevs = libusb_get_device_list( pCtx, &pDevs);
//	printf( "DBG: looping through %d USB devices ...\n", (int)iDevs);
	int iDev=-1;
	for ( int iIdx=0; ((iIdx < iDevs) && (iDev < 0)); iIdx++) {
//		printf( "DBG: device %d ...\n", iIdx+1);
		libusb_device *pDev = pDevs[iIdx];
		libusb_device_handle *pDH=0L;
		int iRC = libusb_open( pDev, &pDH);
		if ( iRC == LIBUSB_SUCCESS) {
			struct libusb_device_descriptor aDev;
			iRC = libusb_get_device_descriptor( pDev, &aDev);
			if ( iRC == LIBUSB_SUCCESS) {
				if ( detectMSiKB( pDH, &aDev)) {
					printf( "DBG: detected MSi KB at %d ...\n", iIdx);
//					showInfo( pDH, &aDev);
					showInfoX( pDev);
					iDev = iIdx;
				}
			}
			libusb_close( pDH);
		} // else printf( "DBG: device open failed\n");
	}
	return;
}

void showInfo( libusb_device_handle *pDev, struct libusb_device_descriptor *pDInf) {
	int iRC=0;
	const int iTxtSz=256;
	unsigned char sTxt[iTxtSz];
//	Get the string associated with iManufacturer
	printf( "DBG:	iManufacturer = %d\n", pDInf->iManufacturer);
	if ( pDInf->iManufacturer > 0) {
		iRC = libusb_get_string_descriptor_ascii(	pDev, pDInf->iManufacturer, sTxt, iTxtSz);
		if ( iRC > 0) {
			printf( "DBG:	manufacturer = %s\n", sTxt);
		} else printf( "DBG: get manufacturer failed\n");
	}
//	Get string associated with iProduct
	printf( "DBG:	iProduct = %d\n", pDInf->iProduct);
	if ( pDInf->iProduct > 0) {
		iRC = libusb_get_string_descriptor_ascii(	pDev, pDInf->iProduct, sTxt, iTxtSz);
		if ( iRC > 0) {
			printf( "DBG:	product = %s\n", sTxt);
		} else printf( "DBG: get product failed\n");
	}
//	Get string associated with iSerialNumber index.
	printf( "DBG:	iSerialNumber = %d\n", pDInf->iSerialNumber);
	if (pDInf->iSerialNumber > 0) {
		iRC = libusb_get_string_descriptor_ascii(	pDev, pDInf->iSerialNumber, sTxt, iTxtSz);
		if (iRC > 0) {
			printf( "DBG:	serialnumber = %s\n", sTxt);
		} else printf( "DBG: get serial number failed\n");
	}
	return;
}

void showInfoX( libusb_device *pDev) {
	struct libusb_device_descriptor aDev;
	if ( libusb_get_device_descriptor( pDev, &aDev) >= 0) {
		printf( "DBG: number of possible configurations: %d\n", (int)aDev.bNumConfigurations);
		printf( "DBG: device class: %d\n", (int)aDev.bDeviceClass);
		printf( "DBG: VendorID: %d\n", aDev.idVendor);
		printf( "DBG: ProductID: %d\n", aDev.idProduct);
		struct libusb_config_descriptor *pCfg;
		libusb_get_config_descriptor( pDev, 0, &pCfg);
		printf( "DBG: number of interfaces: %d\n", (int)pCfg->bNumInterfaces);
		for(int i=0; i<(int)pCfg->bNumInterfaces; i++) {
			const struct libusb_interface *pIF = &pCfg->interface[i];
			printf( "DBG: number of alternate settings: %d\n", pIF->num_altsetting);
			for ( int j=0; j < pIF->num_altsetting; j++) {
				const struct libusb_interface_descriptor *pIFD = &pIF->altsetting[j];
				printf( "DBG: interface number: %d\n", (int)pIFD->bInterfaceNumber);
				printf( "DBG: number of endpoints: %d\n", (int)pIFD->bNumEndpoints);
				for ( int k=0; k < (int)pIFD->bNumEndpoints; k++) {
					const struct libusb_endpoint_descriptor *pEP = &pIFD->endpoint[k];
					printf( "DBG: descriptor type: %d\n", (int)pEP->bDescriptorType);
					printf( "DBG: endpoint addr: %d\n", (int)pEP->bEndpointAddress);
				}
			}
		}
		libusb_free_config_descriptor( pCfg);
	} else printf( "DBG: failed to get device descriptor\n");
	return;
}
