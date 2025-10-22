*** Settings ***
Library    String
Library    SerialLibrary

*** Variables ***
${PORT}      COM7
${BAUD}      115200
${TIMEOUT}   3    # numero sekunteina

*** Test Cases ***
Connect Serial
    Add Port              ${PORT}    baudrate=${BAUD}
    Open Port             ${PORT}
    Port Should Be Open   ${PORT}
    Reset Input Buffer
    Reset Output Buffer

Valid Time String Returns Seconds
    [Documentation]    000120 -> 80
    Write Data         000120\n    encoding=ascii
    ${resp}=           Read Until    terminator=\n    encoding=ascii    timeout=${TIMEOUT}
    ${resp}=           Strip String    ${resp}
    Should Be Equal As Integers    ${resp}    80

Invalid Seconds Are Error
    [Documentation]    001067 -> -6 (sekunnit 0..59)
    Write Data         001067\n    encoding=ascii
    ${resp}=           Read Until    terminator=\n    encoding=ascii    timeout=${TIMEOUT}
    ${resp}=           Strip String    ${resp}
    Should Be Equal As Integers    ${resp}    -6

Non-Digit Is NAN Error
    [Documentation]    12AB56 -> -3 (ei-numero)
    Write Data         12AB56\n    encoding=ascii
    ${resp}=           Read Until    terminator=\n    encoding=ascii    timeout=${TIMEOUT}
    ${resp}=           Strip String    ${resp}
    Should Be Equal As Integers    ${resp}    -3

Length Not Six Is LEN Error
    [Documentation]    01234 -> -2 (pituus ≠ 6)
    Write Data         01234\n    encoding=ascii
    ${resp}=           Read Until    terminator=\n    encoding=ascii    timeout=${TIMEOUT}
    ${resp}=           Strip String    ${resp}
    Should Be Equal As Integers    ${resp}    -2

Disconnect Serial
    Close Port          ${PORT}
    Delete Port         ${PORT}
