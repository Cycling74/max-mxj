{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 0,
			"revision" : 0,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"rect" : [ 74.0, 79.0, 696.0, 515.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"boxes" : [ 			{
				"box" : 				{
					"bgmode" : 0,
					"border" : 0,
					"clickthrough" : 0,
					"embed" : 1,
					"enablehscroll" : 0,
					"enablevscroll" : 0,
					"id" : "obj-72",
					"lockeddragscroll" : 0,
					"maxclass" : "bpatcher",
					"numinlets" : 0,
					"numoutlets" : 4,
					"offset" : [ 0.0, 0.0 ],
					"outlettype" : [ "float", "float", "float", "float" ],
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 8,
							"minor" : 0,
							"revision" : 0,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"rect" : [ 568.0, 117.0, 100.0, 200.0 ],
						"bgcolor" : [ 1.0, 1.0, 1.0, 0.0 ],
						"bglocked" : 0,
						"openinpresentation" : 1,
						"default_fontsize" : 12.0,
						"default_fontface" : 0,
						"default_fontname" : "Arial",
						"gridonopen" : 1,
						"gridsize" : [ 15.0, 15.0 ],
						"gridsnaponopen" : 1,
						"objectsnaponopen" : 1,
						"statusbarvisible" : 2,
						"toolbarvisible" : 1,
						"lefttoolbarpinned" : 0,
						"toptoolbarpinned" : 0,
						"righttoolbarpinned" : 0,
						"bottomtoolbarpinned" : 0,
						"toolbars_unpinned_last_save" : 0,
						"tallnewobj" : 0,
						"boxanimatetime" : 200,
						"enablehscroll" : 1,
						"enablevscroll" : 1,
						"devicewidth" : 0.0,
						"description" : "",
						"digest" : "",
						"tags" : "",
						"style" : "",
						"subpatcher_template" : "",
						"boxes" : [ 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-18",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"patching_rect" : [ 31.308075, 245.162842, 33.0, 22.0 ],
									"presentation_rect" : [ 31.308075, 245.162842, 33.0, 22.0 ],
									"style" : "",
									"text" : "t 64"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-19",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"patching_rect" : [ 77.308075, 245.162842, 25.0, 22.0 ],
									"presentation_rect" : [ 77.308075, 245.162842, 25.0, 22.0 ],
									"style" : "",
									"text" : "t 0"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Courier",
									"fontsize" : 12.0,
									"id" : "obj-20",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 31.308075, 221.147583, 37.0, 20.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 53.308075, 175.147583, 37.0, 20.0 ],
									"style" : "",
									"text" : "zero"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Courier",
									"fontsize" : 12.0,
									"id" : "obj-21",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 77.308075, 221.147583, 37.0, 20.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 53.308075, 153.529266, 37.0, 20.0 ],
									"style" : "",
									"text" : "kill"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-22",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"patching_rect" : [ 192.308075, 245.162842, 33.0, 22.0 ],
									"presentation_rect" : [ 192.308075, 245.162842, 33.0, 22.0 ],
									"style" : "",
									"text" : "t 64"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-23",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"patching_rect" : [ 236.308075, 245.162842, 25.0, 22.0 ],
									"presentation_rect" : [ 236.308075, 245.162842, 25.0, 22.0 ],
									"style" : "",
									"text" : "t 0"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Courier",
									"fontsize" : 12.0,
									"id" : "obj-24",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 192.308075, 221.147583, 37.0, 20.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 53.308075, 127.106873, 37.0, 20.0 ],
									"style" : "",
									"text" : "zero"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Courier",
									"fontsize" : 12.0,
									"id" : "obj-25",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 236.308075, 221.147583, 37.0, 20.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 53.308075, 105.488556, 37.0, 20.0 ],
									"style" : "",
									"text" : "kill"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-26",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"patching_rect" : [ 334.308075, 245.162842, 33.0, 22.0 ],
									"presentation_rect" : [ 334.308075, 245.162842, 33.0, 22.0 ],
									"style" : "",
									"text" : "t 64"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-27",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"patching_rect" : [ 380.308075, 245.162842, 25.0, 22.0 ],
									"presentation_rect" : [ 380.308075, 245.162842, 25.0, 22.0 ],
									"style" : "",
									"text" : "t 0"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Courier",
									"fontsize" : 12.0,
									"id" : "obj-28",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 334.308075, 221.147583, 37.0, 20.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 53.308075, 79.066162, 37.0, 20.0 ],
									"style" : "",
									"text" : "zero"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Courier",
									"fontsize" : 12.0,
									"id" : "obj-29",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 380.308075, 221.147583, 37.0, 20.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 53.308075, 57.447845, 37.0, 20.0 ],
									"style" : "",
									"text" : "kill"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-30",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"patching_rect" : [ 467.308075, 245.162842, 33.0, 22.0 ],
									"presentation_rect" : [ 467.308075, 245.162842, 33.0, 22.0 ],
									"style" : "",
									"text" : "t 64"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-31",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"patching_rect" : [ 522.308105, 245.162842, 25.0, 22.0 ],
									"presentation_rect" : [ 522.308105, 245.162842, 25.0, 22.0 ],
									"style" : "",
									"text" : "t 0"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Courier",
									"fontsize" : 12.0,
									"id" : "obj-32",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 467.308075, 221.147583, 37.0, 20.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 53.308075, 29.824432, 37.0, 20.0 ],
									"style" : "",
									"text" : "zero"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Courier",
									"fontsize" : 12.0,
									"id" : "obj-33",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 522.308105, 221.147583, 37.0, 20.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 53.308075, 8.206108, 37.0, 20.0 ],
									"style" : "",
									"text" : "kill"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-36",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 17.907745, 39.424927, 79.0, 22.0 ],
									"presentation_rect" : [ 17.907745, 39.424927, 79.0, 22.0 ],
									"style" : "",
									"text" : "loadmess 64"
								}

							}
, 							{
								"box" : 								{
									"bgcolor" : [ 1.0, 0.823529, 0.811765, 1.0 ],
									"id" : "obj-37",
									"maxclass" : "dial",
									"needlecolor" : [ 0.360784, 0.360784, 0.360784, 1.0 ],
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "float" ],
									"outlinecolor" : [ 0.65098, 0.666667, 0.662745, 1.0 ],
									"parameter_enable" : 0,
									"patching_rect" : [ 17.63591, 297.328247, 40.0, 40.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 7.63591, 152.328247, 40.0, 40.0 ],
									"style" : ""
								}

							}
, 							{
								"box" : 								{
									"bgcolor" : [ 0.792157, 1.0, 0.615686, 1.0 ],
									"id" : "obj-38",
									"maxclass" : "dial",
									"needlecolor" : [ 0.360784, 0.360784, 0.360784, 1.0 ],
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "float" ],
									"outlinecolor" : [ 0.65098, 0.666667, 0.662745, 1.0 ],
									"parameter_enable" : 0,
									"patching_rect" : [ 166.63591, 297.328247, 40.0, 40.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 7.63591, 104.287537, 40.0, 40.0 ],
									"style" : ""
								}

							}
, 							{
								"box" : 								{
									"bgcolor" : [ 0.74902, 0.901961, 1.0, 1.0 ],
									"id" : "obj-39",
									"maxclass" : "dial",
									"needlecolor" : [ 0.360784, 0.360784, 0.360784, 1.0 ],
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "float" ],
									"outlinecolor" : [ 0.65098, 0.666667, 0.662745, 1.0 ],
									"parameter_enable" : 0,
									"patching_rect" : [ 320.635895, 297.328247, 40.0, 40.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 7.63591, 56.246826, 40.0, 40.0 ],
									"style" : ""
								}

							}
, 							{
								"box" : 								{
									"bgcolor" : [ 1.0, 0.996078, 0.615686, 1.0 ],
									"id" : "obj-40",
									"maxclass" : "dial",
									"needlecolor" : [ 0.360784, 0.360784, 0.360784, 1.0 ],
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "float" ],
									"outlinecolor" : [ 0.65098, 0.666667, 0.662745, 1.0 ],
									"parameter_enable" : 0,
									"patching_rect" : [ 441.635895, 297.328247, 40.0, 40.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 7.63591, 8.206108, 40.0, 40.0 ],
									"style" : ""
								}

							}
, 							{
								"box" : 								{
									"angle" : 0.0,
									"bgcolor" : [ 0.74902, 0.74902, 0.74902, 1.0 ],
									"border" : 2,
									"bordercolor" : [ 0.537255, 0.537255, 0.537255, 1.0 ],
									"id" : "obj-52",
									"maxclass" : "panel",
									"mode" : 0,
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 475.0, 23.0, 100.0, 200.0 ],
									"presentation" : 1,
									"presentation_rect" : [ 0.0, 0.0, 100.0, 200.0 ],
									"proportion" : 0.39,
									"style" : ""
								}

							}
, 							{
								"box" : 								{
									"comment" : "",
									"id" : "obj-68",
									"index" : 4,
									"maxclass" : "outlet",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 441.635895, 360.248108, 25.0, 25.0 ],
									"presentation_rect" : [ 441.635895, 360.248108, 25.0, 25.0 ],
									"style" : ""
								}

							}
, 							{
								"box" : 								{
									"comment" : "",
									"id" : "obj-69",
									"index" : 3,
									"maxclass" : "outlet",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 320.635895, 360.248108, 25.0, 25.0 ],
									"presentation_rect" : [ 320.635895, 360.248108, 25.0, 25.0 ],
									"style" : ""
								}

							}
, 							{
								"box" : 								{
									"comment" : "",
									"id" : "obj-70",
									"index" : 2,
									"maxclass" : "outlet",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 166.63591, 360.248108, 25.0, 25.0 ],
									"presentation_rect" : [ 166.63591, 360.248108, 25.0, 25.0 ],
									"style" : ""
								}

							}
, 							{
								"box" : 								{
									"comment" : "",
									"id" : "obj-71",
									"index" : 1,
									"maxclass" : "outlet",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 17.63591, 360.248108, 25.0, 25.0 ],
									"presentation_rect" : [ 17.63591, 360.248108, 25.0, 25.0 ],
									"style" : ""
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"destination" : [ "obj-37", 0 ],
									"source" : [ "obj-18", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-37", 0 ],
									"source" : [ "obj-19", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-18", 0 ],
									"source" : [ "obj-20", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-19", 0 ],
									"source" : [ "obj-21", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-38", 0 ],
									"source" : [ "obj-22", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-38", 0 ],
									"source" : [ "obj-23", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-22", 0 ],
									"source" : [ "obj-24", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-23", 0 ],
									"source" : [ "obj-25", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-39", 0 ],
									"source" : [ "obj-26", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-39", 0 ],
									"source" : [ "obj-27", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-26", 0 ],
									"source" : [ "obj-28", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-27", 0 ],
									"source" : [ "obj-29", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-40", 0 ],
									"source" : [ "obj-30", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-40", 0 ],
									"source" : [ "obj-31", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-30", 0 ],
									"source" : [ "obj-32", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-31", 0 ],
									"source" : [ "obj-33", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-37", 0 ],
									"order" : 3,
									"source" : [ "obj-36", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-38", 0 ],
									"order" : 2,
									"source" : [ "obj-36", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-39", 0 ],
									"order" : 1,
									"source" : [ "obj-36", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-40", 0 ],
									"order" : 0,
									"source" : [ "obj-36", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-71", 0 ],
									"source" : [ "obj-37", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-70", 0 ],
									"source" : [ "obj-38", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-69", 0 ],
									"source" : [ "obj-39", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-68", 0 ],
									"source" : [ "obj-40", 0 ]
								}

							}
 ]
					}
,
					"patching_rect" : [ 568.0, 117.251907, 100.0, 200.0 ],
					"presentation_rect" : [ 568.0, 117.251907, 100.0, 200.0 ],
					"viewvisibility" : 1
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-58",
					"local" : 1,
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 201.0, 418.0, 45.0, 45.0 ],
					"presentation_rect" : [ 201.0, 418.0, 45.0, 45.0 ],
					"style" : "",
					"varname" : "autohelp_dac"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 11.595187,
					"id" : "obj-59",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 160.0, 424.0, 38.0, 32.0 ],
					"presentation_rect" : [ 160.0, 424.0, 38.0, 32.0 ],
					"style" : "",
					"text" : "start audio",
					"varname" : "autohelp_dac_text"
				}

			}
, 			{
				"box" : 				{
					"angle" : 0.0,
					"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"border" : 2,
					"bordercolor" : [ 0.392157, 0.792157, 0.117647, 1.0 ],
					"id" : "obj-60",
					"maxclass" : "panel",
					"mode" : 0,
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 153.0, 413.0, 100.0, 55.0 ],
					"presentation_rect" : [ 153.0, 413.0, 100.0, 55.0 ],
					"proportion" : 0.39,
					"style" : "",
					"varname" : "startwinwdow_panel"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-1",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"patching_rect" : [ 200.637573, 18.0, 61.0, 22.0 ],
					"presentation_rect" : [ 200.637573, 18.0, 61.0, 22.0 ],
					"style" : "",
					"text" : "loadbang"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-2",
					"linecount" : 3,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 270.525543, 421.837158, 248.0, 47.0 ],
					"presentation_rect" : [ 270.525543, 421.837158, 248.0, 47.0 ],
					"style" : "",
					"text" : "Notice that CPU usage in this patch is substantially lower than if we had 4 seperate instances of [mxj~ biquad ] in a patcher."
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-3",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 37.589787, 183.552185, 145.0, 33.0 ],
					"presentation_rect" : [ 37.589787, 183.552185, 145.0, 33.0 ],
					"style" : "",
					"text" : "min and max gain for all bands in db"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"format" : 6,
					"id" : "obj-4",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 496.741364, 247.109436, 50.0, 20.0 ],
					"presentation_rect" : [ 496.741364, 247.109436, 50.0, 20.0 ],
					"style" : "",
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-5",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 496.741364, 273.531799, 64.0, 22.0 ],
					"presentation_rect" : [ 496.741364, 273.531799, 64.0, 22.0 ],
					"style" : "",
					"text" : "hifreq $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"format" : 6,
					"id" : "obj-6",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 405.802307, 247.109436, 50.0, 20.0 ],
					"presentation_rect" : [ 405.802307, 247.109436, 50.0, 20.0 ],
					"style" : "",
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-7",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 405.802307, 273.531799, 85.0, 22.0 ],
					"presentation_rect" : [ 405.802307, 273.531799, 85.0, 22.0 ],
					"style" : "",
					"text" : "himidfreq $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"format" : 6,
					"id" : "obj-8",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 316.181213, 247.109436, 50.0, 20.0 ],
					"presentation_rect" : [ 316.181213, 247.109436, 50.0, 20.0 ],
					"style" : "",
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-9",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 316.181213, 273.531799, 85.0, 22.0 ],
					"presentation_rect" : [ 316.181213, 273.531799, 85.0, 22.0 ],
					"style" : "",
					"text" : "lomidfreq $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"format" : 6,
					"id" : "obj-10",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 243.693573, 247.109436, 50.0, 20.0 ],
					"presentation_rect" : [ 243.693573, 247.109436, 50.0, 20.0 ],
					"style" : "",
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-11",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 243.693573, 273.531799, 64.0, 22.0 ],
					"presentation_rect" : [ 243.693573, 273.531799, 64.0, 22.0 ],
					"style" : "",
					"text" : "lofreq $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"format" : 6,
					"id" : "obj-12",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 112.257004, 225.109436, 50.0, 20.0 ],
					"presentation_rect" : [ 112.257004, 225.109436, 50.0, 20.0 ],
					"style" : "",
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"format" : 6,
					"id" : "obj-13",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 25.271828, 225.109436, 50.0, 20.0 ],
					"presentation_rect" : [ 25.271828, 225.109436, 50.0, 20.0 ],
					"style" : "",
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-14",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 112.257004, 251.531799, 79.0, 22.0 ],
					"presentation_rect" : [ 112.257004, 251.531799, 79.0, 22.0 ],
					"style" : "",
					"text" : "maxgain $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-15",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 25.271828, 251.531799, 74.0, 22.0 ],
					"presentation_rect" : [ 25.271828, 251.531799, 74.0, 22.0 ],
					"style" : "",
					"text" : "mingain $1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 18.0,
					"id" : "obj-16",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 11.0, 7.0, 106.0, 27.0 ],
					"presentation_rect" : [ 11.0, 7.0, 106.0, 27.0 ],
					"style" : "",
					"text" : "mxj~ djEq",
					"textcolor" : [ 0.101961, 0.031373, 0.223529, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-41",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 373.60791, 83.674286, 20.0, 20.0 ],
					"presentation_rect" : [ 373.60791, 83.674286, 20.0, 20.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-42",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 373.60791, 114.900757, 103.0, 22.0 ],
					"presentation_rect" : [ 373.60791, 114.900757, 103.0, 22.0 ],
					"style" : "",
					"text" : "benchmark $1 64"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-43",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 200.673798, 384.038177, 50.0, 22.0 ],
					"presentation_rect" : [ 200.673798, 384.038177, 50.0, 22.0 ],
					"style" : "",
					"text" : "*~ 0.6"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-45",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 235.698517, 194.699738, 71.0, 22.0 ],
					"presentation_rect" : [ 235.698517, 194.699738, 71.0, 22.0 ],
					"style" : "",
					"text" : "viewsource"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-46",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 228.181213, 65.251892, 35.0, 22.0 ],
					"presentation_rect" : [ 228.181213, 65.251892, 35.0, 22.0 ],
					"style" : "",
					"text" : "open"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-48",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 220.766052, 111.473297, 20.0, 20.0 ],
					"presentation_rect" : [ 220.766052, 111.473297, 20.0, 20.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-49",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 201.319611, 43.633606, 128.0, 22.0 ],
					"presentation_rect" : [ 201.319611, 43.633606, 128.0, 22.0 ],
					"style" : "",
					"text" : "open social.aif, loop 1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-50",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "bang" ],
					"patching_rect" : [ 200.766052, 139.101776, 66.0, 22.0 ],
					"presentation_rect" : [ 200.766052, 139.101776, 66.0, 22.0 ],
					"saved_object_attributes" : 					{
						"basictuning" : 440,
						"followglobaltempo" : 0,
						"formantcorrection" : 0,
						"mode" : "basic",
						"originallength" : [ 3686.378231, "ticks" ],
						"originaltempo" : 120.0,
						"pitchcorrection" : 0,
						"quality" : "basic",
						"timestretch" : [ 0 ]
					}
,
					"style" : "",
					"text" : "sfplay~ 1"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-51",
					"maxclass" : "newobj",
					"numinlets" : 5,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "" ],
					"patching_rect" : [ 200.827011, 339.755707, 426.0, 22.0 ],
					"presentation_rect" : [ 200.827011, 339.755707, 426.0, 22.0 ],
					"style" : "",
					"text" : "mxj~ djEq @lofreq 60 @lomidfreq 500 @himidfreq 2200 @hifreq 10000"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-53",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 299.693573, 225.552185, 183.0, 20.0 ],
					"presentation_rect" : [ 299.693573, 225.552185, 183.0, 20.0 ],
					"style" : "",
					"text" : "center frequencies for EQ bands"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-54",
					"linecount" : 3,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 270.525543, 370.19339, 222.0, 47.0 ],
					"presentation_rect" : [ 270.525543, 370.19339, 222.0, 47.0 ],
					"style" : "",
					"text" : "In this example we leverage the code we wrote for the mxj~ biquad class to compose a higher level perform routine."
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-55",
					"linecount" : 8,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 528.229004, 366.480957, 129.0, 114.0 ],
					"presentation_rect" : [ 528.229004, 366.480957, 129.0, 114.0 ],
					"style" : "",
					"text" : "By maximizing the the amount of work we do in Java per perform routine we minimize the overhead of the C/Java boundry crossings and thus increase performance"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-56",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 11.0, 33.834606, 93.0, 20.0 ],
					"presentation_rect" : [ 11.0, 33.834606, 93.0, 20.0 ],
					"style" : "",
					"text" : "by topher lafata"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-57",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 242.355835, 110.85495, 59.0, 20.0 ],
					"presentation_rect" : [ 242.355835, 110.85495, 59.0, 20.0 ],
					"style" : "",
					"text" : "play/stop"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-49", 0 ],
					"midpoints" : [ 210.137573, 39.430023, 210.819611, 39.430023 ],
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-11", 0 ],
					"source" : [ "obj-10", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"midpoints" : [ 253.193573, 315.143738, 210.327011, 315.143738 ],
					"source" : [ "obj-11", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-14", 0 ],
					"source" : [ "obj-12", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-15", 0 ],
					"source" : [ "obj-13", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"hidden" : 1,
					"midpoints" : [ 121.757004, 315.143738, 210.327011, 315.143738 ],
					"source" : [ "obj-14", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"hidden" : 1,
					"midpoints" : [ 34.771828, 330.143738, 210.327011, 330.143738 ],
					"source" : [ "obj-15", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-5", 0 ],
					"source" : [ "obj-4", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-42", 0 ],
					"source" : [ "obj-41", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"midpoints" : [ 383.10791, 182.544525, 210.327011, 182.544525 ],
					"source" : [ "obj-42", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-58", 1 ],
					"order" : 0,
					"source" : [ "obj-43", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-58", 0 ],
					"order" : 1,
					"source" : [ "obj-43", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"midpoints" : [ 245.198517, 223.333328, 210.327011, 223.333328 ],
					"source" : [ "obj-45", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-50", 0 ],
					"midpoints" : [ 237.681213, 94.694656, 210.266052, 94.694656 ],
					"source" : [ "obj-46", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-50", 0 ],
					"source" : [ "obj-48", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-50", 0 ],
					"midpoints" : [ 210.819611, 122.694656, 210.266052, 122.694656 ],
					"source" : [ "obj-49", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"midpoints" : [ 506.241364, 315.143738, 210.327011, 315.143738 ],
					"source" : [ "obj-5", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"midpoints" : [ 210.266052, 250.428741, 210.327011, 250.428741 ],
					"source" : [ "obj-50", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-43", 0 ],
					"source" : [ "obj-51", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-7", 0 ],
					"source" : [ "obj-6", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"midpoints" : [ 415.302307, 315.143738, 210.327011, 315.143738 ],
					"source" : [ "obj-7", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 4 ],
					"hidden" : 1,
					"source" : [ "obj-72", 3 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 3 ],
					"hidden" : 1,
					"source" : [ "obj-72", 2 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 2 ],
					"hidden" : 1,
					"source" : [ "obj-72", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 1 ],
					"hidden" : 1,
					"source" : [ "obj-72", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"source" : [ "obj-8", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"midpoints" : [ 325.681213, 315.143738, 210.327011, 315.143738 ],
					"source" : [ "obj-9", 0 ]
				}

			}
 ],
		"dependency_cache" : [  ],
		"autosave" : 0
	}

}
