function arduino = importArduinoFile(filename, dataLines)
%IMPORTFILE Import data from a text file
%  Arduino = IMPORTARDUINOFILE(FILENAME) reads data from text
%  file FILENAME for the default selection.  Returns the data as a table.
%
%  DMM = IMPORTARDUINOFILE(FILE, DATALINES) reads data for
%  the specified row interval(s) of text file FILENAME. Specify
%  DATALINES as a positive scalar integer or a N-by-2 array of positive
%  scalar integers for dis-contiguous row intervals.
%
%  Example:
%  Arduino = importarduinofile(filename, [1, Inf]);
%
%  See also READTABLE.

%% Input handling

% If dataLines is not specified, define defaults
if nargin < 2
    dataLines = [1, Inf];
end

%% Setup the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 38);

% Specify range and delimiter
opts.DataLines = dataLines;
opts.Delimiter = ["\t", " ", ",", "A"]; 

% Specify column names and types
opts.VariableNames = ["Time", "Var2", "Var3", "SetPoint", "Var5", "CurrentSetPosition", "Var7", "Var8", "PID_Error", "Var10", "PID", "Var12", "P", "Var14", "I", "Var16", "D", "Var18", "Wash_Temp", "Var20", "Outlet_Temp", "Var22", "Var23", "Mass", "Var25", "Var26", "Mass_Delta", "Var28", "Frequency", "Var30", "Tower_Temp", "Var32", "SetTemp", "Var34", "SetTempCounter", "Var36", "CheckPoint", "Var38", "Anomoly"];
opts.SelectedVariableNames = ["Time", "SetPoint", "CurrentSetPosition", "PID_Error", "PID", "P", "I", "D", "Wash_Temp", "Outlet_Temp", "Mass", "Mass_Delta", "Frequency", "Tower_Temp", "SetTemp", "SetTempCounter", "CheckPoint", "Anomoly"];
opts.VariableTypes = ["datetime", "string", "string", "double", "string", "double", "string", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "string", "double", "string", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double"];

% Specify file level properties
opts.ImportErrorRule = "omitrow";
opts.MissingRule = "omitrow";
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Specify variable properties
opts = setvaropts(opts, ["Var2", "Var3", "Var5", "Var7", "Var8", "Var10", "Var12", "Var14", "Var16", "Var18", "Var20", "Var22", "Var23", "Var25", "Var26", "Var28", "Var30", "Var32", "Var34", "Var36"], "WhitespaceRule", "preserve");
opts = setvaropts(opts, ["Var2", "Var3", "Var5", "Var7", "Var8", "Var10", "Var12", "Var14", "Var16", "Var18", "Var20", "Var22", "Var23", "Var25", "Var26", "Var28", "Var30", "Var32", "Var34", "Var36"], "EmptyFieldRule", "auto");
opts = setvaropts(opts, "Time", "InputFormat", "HH:mm:ss.SSS");

% Import the data
arduino = readtable(filename, opts);

clear opts
end