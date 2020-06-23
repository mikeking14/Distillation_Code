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
opts = delimitedTextImportOptions("NumVariables", 33);

% Specify range and delimiter
opts.DataLines = dataLines;
opts.Delimiter = ["\t",">" ","]; 

% Specify column names and types
opts.VariableNames = ["Time", "Var2", "SetPoint", "Var4", "CurrentSetPosition", "Var6", "PID_Error", "Var8", "PID", "Var10", "P", "Var12", "I", "Var14", "D", "Var16", "Wash_Temp", "Var18", "Outlet_Temp", "Var20", "Mass", "Var22", "Mass_Delta", "Var24", "Frequency", "Var26", "Tower_Temp", "Var28", "Set_Temp", "Var30", "Set_Temp_Counter", "Var32", "CheckPoint"];
opts.SelectedVariableNames = ["Time", "SetPoint", "CurrentSetPosition", "PID_Error", "PID", "P", "I", "D", "Wash_Temp", "Outlet_Temp", "Mass", "Mass_Delta", "Frequency", "Tower_Temp", "Set_Temp", "Set_Temp_Counter", "CheckPoint"];
opts.VariableTypes = ["datetime", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double"];

% Specify file level properties
opts.ImportErrorRule = "omitrow";
opts.MissingRule = "omitrow";
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Specify variable properties
opts = setvaropts(opts, ["Var2", "Var4", "Var6", "Var8", "Var10", "Var12", "Var14", "Var16", "Var18", "Var20", "Var22", "Var24", "Var26", "Var28", "Var30", "Var32"], "WhitespaceRule", "preserve");
opts = setvaropts(opts, ["Var2", "Var4", "Var6", "Var8", "Var10", "Var12", "Var14", "Var16", "Var18", "Var20", "Var22", "Var24", "Var26", "Var28", "Var30", "Var32"], "EmptyFieldRule", "auto");
opts = setvaropts(opts, "Time", "InputFormat", "HH:mm:ss.SSS");

% Import the data
arduino = readtable(filename, opts);

clear opts
end