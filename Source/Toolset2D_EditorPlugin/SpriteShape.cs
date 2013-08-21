using System;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;
using System.Drawing.Design;
using System.Collections.Generic;
using CSharpFramework;
using CSharpFramework.Math;
using CSharpFramework.Shapes;
using CSharpFramework.Actions;
using CSharpFramework.PropertyEditors;
using CSharpFramework.DynamicProperties;
using CSharpFramework.UndoRedo;
using CSharpFramework.Scene;
using CSharpFramework.View;
using CSharpFramework.Serialization;
using Toolset2D_Managed;
using System.Runtime.Serialization;
using ManagedFramework;
using System.IO;

namespace Toolset2D
{
    public class IconManager
    {
        public static int m_SpriteIndex = -1;
        public static int SpriteIndex
        {
            get
            {
                if (m_SpriteIndex == -1)
                {
                    m_SpriteIndex = EditorManager.GUI.ShapeTreeImages.AddBitmap(Toolset2D.Resources.sprite, "SpriteIcon", Color.Magenta);
                }
                return m_SpriteIndex;
            }
        }
    }

    #region class SpriteShape
    /// <summary>
    /// SpriteShape : This is the class that represents the shape in the editor. It has an engine instance that handles the
    /// native code. The engine instance code in located in the Toolset2D_Managed project (managed C++ class library)
    /// </summary>
    [Serializable]
    public class SpriteShape : Shape3D
    {
        /// <summary>
        /// Category string
        /// </summary>
        protected const string CAT_EVENTRES = "Sprite";

        /// <summary>
        /// Category ID
        /// </summary>
        protected const int CATORDER_EVENTRES = Shape3D.LAST_CATEGORY_ORDER_ID + 1;

        #region Constructor

        /// <summary>
        /// The constructor of the node shape, just takes the node name
        /// </summary>
        /// <param name="name">Name of the shape in the shape tree</param>
        public SpriteShape(string name)
            : base(name)
        {
        }

        #endregion

        #region Icon

        /// <summary>
        /// Return the icon index in the image list for the shape tree view. For this, use a static variable for this class.
        /// </summary>
        public override int GetIconIndex()
        {
            return IconManager.SpriteIndex;
        }

        /// <summary>
        /// Gets the icon index for selected icons in the tree view. Simply pass the icon index
        /// </summary>
        /// <returns></returns>
        public override int GetSelectedIconIndex()
        {
            return IconManager.SpriteIndex;
        }

        #endregion

        #region Engine Instance

        /// <summary>
        /// Function to create the engine instance of the shape. The engine instance is of type EngineInstanceSprite
        /// and located in the managed code library.
        /// </summary>
        /// <param name="bCreateChildren">relevant for the base class to create instances for children</param>
        public override void CreateEngineInstance(bool bCreateChildren)
        {
            base.CreateEngineInstance(bCreateChildren);
            _engineInstance = new EngineInstanceSprite();
            SetEngineInstanceBaseProperties(); // sets the position etc.
        }

        /// <summary>
        /// Removes the engine instance
        /// </summary>
        /// <param name="bRemoveChildren">relevant for the base class to cleanup its children</param>
        public override void RemoveEngineInstance(bool bRemoveChildren)
        {
            base.RemoveEngineInstance(bRemoveChildren);

            // nothing else to do here (_engineInstance already destroyed in base.RemoveEngineInstance)
        }

        /// <summary>
        /// Helper field to access the engine instance as casted class to perform specfic operations on it
        /// </summary>
        EngineInstanceSprite EngineNode { get { return (EngineInstanceSprite)_engineInstance; } }

        /// <summary>
        /// Overridden render function: Let the engine instance render itself and render a box
        /// </summary>
        /// <param name="view"></param>
        /// <param name="mode"></param>
        public override void RenderShape(VisionViewBase view, ShapeRenderMode mode)
        {
            if (HasEngineInstance())
            {
                EngineNode.RenderShape(view, mode);
                base.RenderShape(view, mode);
            }
        }

        /// <summary>
        /// Set all properties on the engine instance. The base implementation already sets position and orientation,
        /// so there is nothing else to do here
        /// </summary>
        public override void SetEngineInstanceBaseProperties()
        {
            base.SetEngineInstanceBaseProperties();

            if (HasEngineInstance())
            {
                EngineNode.SetShoeBoxData(m_SpriteSheetFilename, m_ShoeBoxFilename);
            }
        }

        /// <summary>
        /// Pick the shape in the view. Use helper function trace the local bounding box.
        /// </summary>
        /// <param name="rayStart">trace ray start</param>
        /// <param name="rayEnd">trace ray end</param>
        /// <param name="result">esult structure to fill in</param>
        public override void OnTraceShape(ShapeTraceMode_e mode, Vector3F rayStart, Vector3F rayEnd, ref ShapeTraceResult result)
        {
            if (ConversionUtils.TraceOrientedBoundingBox(LocalBoundingBox, Position, RotationMatrix, rayStart, rayEnd, ref result))
            {
                result.hitShape = this;
            }
        }        
        #endregion
        
        #region Serialization and Export

        /// <summary>
        /// Called when deserializing
        /// </summary>
        /// <param name="info"></param>
        /// <param name="context"></param>
        protected SpriteShape(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
            if (SerializationHelper.HasElement(info, "_sheet_filename"))
            {
                SpriteSheetFilename = info.GetString("_sheet_filename");
                ShoeBoxData = info.GetString("_shoebox_filename");
            }
        }

        /// <summary>
        /// Called during export to collect native plugin information. In this case, return the global instance that applies for all shapes in this plugin
        /// </summary>
        /// <returns></returns>
        public override CSharpFramework.Serialization.EditorPluginInfo GetPluginInformation()
        {
            return EditorPlugin.EDITOR_PLUGIN_INFO;
        }

        /// <summary>
        /// Called when serializing
        /// </summary>
        /// <param name="info"></param>
        /// <param name="context"></param>
        public override void GetObjectData(System.Runtime.Serialization.SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
        {
            base.GetObjectData(info, context);
            info.AddValue("_sheet_filename", m_SpriteSheetFilename);
            info.AddValue("_shoebox_filename", m_ShoeBoxFilename);
        }

        /// <summary>
        /// Called when exporting the scene to engine archive. base implementation calls function on engine object which in turn
        /// serializes itself
        /// </summary>
        /// <returns></returns>
        public override bool OnExport(CSharpFramework.Scene.SceneExportInfo info)
        {
            return base.OnExport(info);
        }

        #endregion

        #region Properties
        string m_SpriteSheetFilename;
        string m_ShoeBoxFilename;

        [SortedCategory(CAT_EVENTRES, CATORDER_EVENTRES), PropertyOrder(1)]
        [EditorAttribute(typeof(AssetEditor), typeof(UITypeEditor)), AssetDialogFilter(new string[] { "Texture" })]
        [Description("Texture used for the sprite sheet.")]
        public string SpriteSheetFilename
        {
            get { return m_SpriteSheetFilename; }
            set
            {
                if (m_SpriteSheetFilename == value) return;
                m_SpriteSheetFilename = value;
                SetEngineInstanceBaseProperties();
            }
        }

        [PrefabResolveFilename]
        [SortedCategory(CAT_EVENTRES, CATORDER_EVENTRES),
        PropertyOrder(1),
        EditorAttribute(typeof(FilenameEditor), typeof(UITypeEditor)),
        FileDialogFilter(new string[] { ".xml" })]
        [Description("XML file output by ShoeBox that describes where the cells are in the sprite sheet")]
        [DependentProperties(new string[] { "SpriteSheetFilename" })]
        public string ShoeBoxData
        {
            get { return m_ShoeBoxFilename; }
            set
            {
                if (m_ShoeBoxFilename == value) return;
                m_ShoeBoxFilename = value;
                SetEngineInstanceBaseProperties();
            }
        }
        #endregion
    }
    #endregion

    #region Shape Creator Plugin


    /// <summary>
    /// Creator class. An instance of the creator is registered in the plugin init function. Thus the creator shows
    /// up in the "Create" menu of the editor
    /// </summary>
    class SpriteShapeCreator : CSharpFramework.IShapeCreatorPlugin
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public SpriteShapeCreator()
        {
            IconIndex = IconManager.SpriteIndex;
            CategoryIconIndex = IconManager.SpriteIndex;
        }

        /// <summary>
        /// Get the name of the plugin, for instance the shape name. This name appears in the "create" menu
        /// </summary>
        /// <returns>creator name</returns>
        public override string GetPluginName()
        {
            return "2D Sprite";
        }

        /// <summary>
        /// Get the plugin category name to sort the plugin name. This is useful to group m_shapeCreators. A null string can
        /// be returned to put the creator in the root
        /// </summary>
        /// <returns></returns>
        public override string GetPluginCategory()
        {
            return "2D Toolset";
        }

        /// <summary>
        /// Returns a short description text
        /// </summary>
        /// <returns></returns>
        public override string GetPluginDescription()
        {
            return "2D Sprite Entity";
        }

        public override ShapeBase CreateShapeInstance()
        {
            SpriteShape shape = new SpriteShape("Sprite");
            shape.Position = EditorManager.Scene.CurrentShapeSpawnPosition;
            return shape;
        }
    }

    #endregion
}