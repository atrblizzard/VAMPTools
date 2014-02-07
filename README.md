VAMPTools
=============

A toolset for porting Vampire the Masquerade: Bloodlines models and textures to FBX written by Kiyoshi555

Requirements
-----------------------------------------------
- Bloodlines SDK:
http://www.planetvampire.com/modules/files/view.php?id=764

- Grab the latest Autodesk FBX 2013 SDK from there:
http://usa.autodesk.com/adsk/servlet/pc/item?siteID=123112&id=21795241

- DevIL SDK:
http://openil.sourceforge.net/download.php

Installation
----------------------------------------

- Extract DeVIL SDK into the newly created DevIL folder inside the project folder.
- Bloodlines SDK is required to be installed into the original location of Vampire the Masquerade: Bloodlines. After running the SDK launcher the user is prompted to extract all contents from the game to allow the tool to convert the models into FBX files.

Usage:
-----------------------------------------
MDLConverter [opts] [file(s)]

Bloodlines SDK is required extract all contents from Vampire the Masquerade: Bloodlines to be used by the MDLConverter tool. The contents must be in the vampire path (usually in Vampire the Masquerade - Bloodlines\SDKContents\vampire) in order to convert the models and textures. 

Third party libraries used:
----------------------------
Autodesk FBX SDK 2013.3
DevIL
zlib


Libraries:
----------------------------
- Source SDK (modified) - http://developer.valvesoftware.com/
- VTFLib - http://nemesis.thewavelength.net/index.php?p=40 
- DevIL - http://openil.sourceforge.net/
- zlib - http://zlib.net/ 
- Autodesk FBX SDK 2013: http://usa.autodesk.com/

Changes to libraries available upon request as per GPL/LPGL licenses (where applicable)

License:
----------------------------
Creative Commons Attribution NonCommercial NoDerivs (CC-NC-ND)

The most restrictive creative commons license. This only allows people to download and share your work for no commercial gain and for no other purposes. You must give credit to the original author of the work, state their name and the title of the original work, and include the attribution logo found on their website here. Doesn’t allow for Tivoization and provides protection from defamation for the creator.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
