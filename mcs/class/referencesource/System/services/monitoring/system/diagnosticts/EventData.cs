using System.ComponentModel;
using System.Security.Permissions;
using System.Security;

namespace System.Diagnostics {

    public class EventInstance {
        private int _categoryNumber;
        private EventLogEntryType _entryType = EventLogEntryType.Information;
        private long _instanceId;

        public EventInstance(long instanceId, int categoryId) {
            CategoryId = categoryId;
            InstanceId = instanceId;
        }

        public EventInstance(long instanceId, int  categoryId, EventLogEntryType entryType) : this (instanceId, categoryId) {
            EntryType = entryType;
        }

        public int CategoryId {
            get { return _categoryNumber; }
            set {
                if (value > UInt16.MaxValue || value < 0)
                    throw new ArgumentOutOfRangeException("value");

                _categoryNumber = value;
            }
        }

        public EventLogEntryType EntryType {
            get { return _entryType; }

            set {
                if (!Enum.IsDefined(typeof(EventLogEntryType), value))
                    throw new InvalidEnumArgumentException("value", (int)value, typeof(EventLogEntryType));
                
                _entryType = value;
            }
        }

        public long InstanceId {
            get { return _instanceId; }
            set {
                if (value > UInt32.MaxValue || value < 0)
                    throw new ArgumentOutOfRangeException("value");

                _instanceId = value;
            }
        }
    }
}
