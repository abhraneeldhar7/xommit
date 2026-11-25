import { Cancel01Icon, CheckmarkCircle01Icon, InformationCircleIcon, MinusSignIcon, SquareIcon } from "hugeicons-react";
import { XingIcon } from "hugeicons-react";

export const icons = {
  check: CheckmarkCircle01Icon,
  Close: Cancel01Icon,
  Minus: MinusSignIcon,
  Square: SquareIcon,
  info: InformationCircleIcon
} as const;

export type IconName = keyof typeof icons;